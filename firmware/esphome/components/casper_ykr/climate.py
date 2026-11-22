import esphome.codegen as cg
from esphome.components import climate_ir
from esphome.types import ConfigType

AUTO_LOAD = ["climate_ir"]

casper_ykr_ns = cg.esphome_ns.namespace("casper_ykr")
CasperYkrClimate = casper_ykr_ns.class_("CasperYkrClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(CasperYkrClimate)


async def to_code(config: ConfigType) -> None:
    await climate_ir.new_climate_ir(config)
