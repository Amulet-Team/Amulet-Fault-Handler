from __future__ import annotations

__all__: list[str] = [
    "throw_access_violation",
    "throw_heap_corruption",
    "throw_stack_overflow",
]

def throw_access_violation() -> None: ...
def throw_heap_corruption() -> None: ...
def throw_stack_overflow() -> None: ...
