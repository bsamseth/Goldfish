from enum import Enum
from pathlib import Path
from typing import Annotated, Any

from pydantic import BaseModel
from pydantic.functional_validators import BeforeValidator


class RangeKind(Enum):
    Integer = "Integer"


class RangeConfig(BaseModel):
    kind: RangeKind
    start: float
    std: float

    @classmethod
    def from_str(cls, s: str) -> "RangeConfig":
        kind, rest = s.split("(")
        start, std = rest.rstrip(")").split(",")
        return cls(kind=RangeKind(kind), start=float(start), std=float(std))


ParameterRanges = dict[
    str, Annotated[RangeConfig, BeforeValidator(RangeConfig.from_str)]
]


class Config(BaseModel):
    engine: "EngineConfig"
    parameter_ranges: ParameterRanges
    log_csv_path: Path
    book_path: Path
    syzygy_path: Path
    concurrency: int
    n_generations: int
    apply_factor: float = 0.002


class EngineConfig(BaseModel):
    command: str
    fixed_parameters: dict[str, Any]
