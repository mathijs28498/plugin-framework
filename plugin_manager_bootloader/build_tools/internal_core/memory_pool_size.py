from internal_core.datatypes import AppConfig
import ast
import operator
from typing import Optional, Union, Any
from dataclasses import dataclass
from enum import Enum

PLUGIN_MEMORY_SLAB_SIZE = 1024
MAX_INT32 = (1 << 31) - 1
MAX_MATH_EVALUATION_ITERATIONS = 32


class AstEvalNodeType(Enum):
    INT = 1
    ADD = 2
    SUB = 3
    MUL = 4
    DIV = 5
    NEG = 6


ALLOWED_OPERATORS = {
    ast.Add: AstEvalNodeType.ADD,
    ast.Sub: AstEvalNodeType.SUB,
    ast.Mult: AstEvalNodeType.MUL,
    ast.Div: AstEvalNodeType.DIV,
}

AST_EVAL_NODE_TYPE_TO_OPERATOR = {
    AstEvalNodeType.ADD: operator.add,
    AstEvalNodeType.SUB: operator.sub,
    AstEvalNodeType.MUL: operator.mul,
    AstEvalNodeType.DIV: operator.floordiv,
}

AST_NAMED_VALUES = {
    "kib": 1024,
    "mib": 1024**2,
}


@dataclass
class AstEvalNode:
    type: AstEvalNodeType
    value: Optional[int]
    child_indices: Optional[tuple[int, Optional[int]]]


def evaluate_math_expression_string(expression: int | str) -> int:
    if isinstance(expression, int):
        return expression

    expression_module = ast.parse(expression, mode="eval")

    nodes: list[Union[ast.expr, AstEvalNode]] = [expression_module.body]
    # This loop breaks if it hits the final leave node.
    # It allows for a total of MAX_MATH_EVALUATION_ITERATIONS nodes to be evaluated before hitting the error limit
    for i in range(MAX_MATH_EVALUATION_ITERATIONS):
        if len(nodes) == i:
            break

        node = nodes[i]
        if isinstance(node, ast.Constant):
            if type(node.value) is not int:
                raise ValueError(
                    f"Node '{node.value}' constant is not an integer, it is {type(node.value)}: {expression}"
                )

            nodes[i] = AstEvalNode(
                type=AstEvalNodeType.INT,
                value=node.value,
                child_indices=None,
            )

            continue

        if isinstance(node, ast.Name):
            value = AST_NAMED_VALUES.get(node.id.lower(), None)
            if value is None:
                raise ValueError(f"Unknown named value '{node.id}': {expression}")

            nodes[i] = AstEvalNode(
                type=AstEvalNodeType.INT,
                value=value,
                child_indices=None,
            )

            continue

        if isinstance(node, ast.UnaryOp):
            if not isinstance(node.op, ast.USub):
                raise ValueError(
                    f"Node of type '{type(node.op)}' is not allowed: {expression}"
                )

            nodes[i] = AstEvalNode(
                type=AstEvalNodeType.NEG,
                value=None,
                child_indices=(len(nodes), None),
            )
            nodes.append(node.operand)
            continue

        if isinstance(node, ast.BinOp):
            operator_type = ALLOWED_OPERATORS.get(type(node.op), None)

            if operator_type is None:
                raise ValueError(
                    f"Node of type '{type(node.op)}' is not allowed: {expression}"
                )

            nodes[i] = AstEvalNode(
                type=operator_type,
                value=None,
                child_indices=(len(nodes), len(nodes) + 1),
            )
            nodes.append(node.left)
            nodes.append(node.right)
            continue

        node_text = f"{node}"
        if isinstance(node, ast.expr):
            node_text = ast.dump(node)

        raise ValueError(f"Unknown math node '{node_text}': {expression}")

    else:
        raise ValueError(
            f"Reached max evaluation limit ({MAX_MATH_EVALUATION_ITERATIONS}) for evaluating math expression: {expression}"
        )

    # Based on the limited subset of expressions allowed, the nodes are sorted in order
    # Evaluating this list in reverse order makes it so the math gets evaluated from the leaves up
    for node in reversed(nodes):
        if isinstance(node, ast.expr):
            raise ValueError(
                f"Node '{ast.dump(node)}' did not get evaluated properly: {expression}"
            )

        if node.type == AstEvalNodeType.INT:
            continue

        if node.child_indices is None:
            raise ValueError(f"Node is operand without children '{node}': {expression}")

        node_left = nodes[node.child_indices[0]]
        if isinstance(node_left, ast.expr):
            raise ValueError(
                f"Left node of operand did not get evaluated properly '{ast.dump(node_left)}': {expression}"
            )

        if node_left.type != AstEvalNodeType.INT or node_left.value is None:
            raise ValueError(
                f"Left node of operand is not an INT type '{node_left}': {expression}"
            )

        if node.type == AstEvalNodeType.NEG:
            node.value = -node_left.value
            node.type = AstEvalNodeType.INT
            continue

        if node.child_indices[1] is None:
            raise ValueError(f"Binary operand has only 1 child '{node}': {expression}")

        node_right = nodes[node.child_indices[1]]
        if isinstance(node_right, ast.expr):
            raise ValueError(
                f"Right node of operand did not get evaluated properly '{ast.dump(node_right)}': {expression}"
            )

        if node_right.type != AstEvalNodeType.INT or node_right.value is None:
            raise ValueError(
                f"Right node of operand is not an INT type '{node_right}': {expression}"
            )

        if node.type == AstEvalNodeType.DIV and node_right.value == 0:
            raise ValueError(f"Cannot divide by 0: {expression}")

        node.value = AST_EVAL_NODE_TYPE_TO_OPERATOR[node.type](
            node_left.value, node_right.value
        )
        node.type = AstEvalNodeType.INT

    root_node = nodes[0]

    if (
        isinstance(root_node, ast.expr)
        or root_node.type != AstEvalNodeType.INT
        or root_node.value is None
    ):
        raise ValueError(f"Error with evaluated root node: {root_node}: {expression}")

    return root_node.value


def calculate_memory_pool_size(app_config: AppConfig) -> int:
    memory_pool_size = app_config.max_plugin_amount * PLUGIN_MEMORY_SLAB_SIZE

    memory_pool_size += app_config.memory_arena_size

    if memory_pool_size > MAX_INT32:
        raise ValueError(
            f"Memory pool size is too large, is '{memory_pool_size}', max is '{MAX_INT32}'"
        )

    return memory_pool_size
