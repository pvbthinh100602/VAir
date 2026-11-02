import esphome.codegen as cg
from esphome.components import climate_ir
from esphome.types import ConfigType

AUTO_LOAD = ["climate_ir"]

daikin_ftkb_ns = cg.esphome_ns.namespace("daikin_ftkb")
DaikinFtkbClimate = daikin_ftkb_ns.class_("DaikinFtkbClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(DaikinFtkbClimate)


async def to_code(config: ConfigType) -> None:
    await climate_ir.new_climate_ir(config)
