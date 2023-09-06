import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
# from esphome.components import mqtt

DEPENDENCIES = ["network", "time"]
# AUTO_LOAD = ["mqtt"]
# MULTI_CONF = True

tuya_ns = cg.esphome_ns.namespace('tuya_iot_component')

TuyaComponent = tuya_ns.class_('TuyaIotComponent', cg.PollingComponent)
TimeComponent = cg.esphome_ns.namespace('homeassistant').class_('HomeassistantTime')

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(TuyaComponent),
    cv.GenerateID("time_id"): cv.use_id(TimeComponent),
    cv.Required("product_id"): cv.string,
    cv.Required("device_id"): cv.string,
    cv.Required("device_secret"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    timeComponent = yield cg.get_variable(config["time_id"])
    cg.add(var.set_time(timeComponent))
    cg.add(var.set_product_id(config["product_id"]))
    cg.add(var.set_device_id(config["device_id"]))
    cg.add(var.set_device_secret(config["device_secret"]))
    cg.add_library("daknuett/cryptosuite2", "0.2.7")