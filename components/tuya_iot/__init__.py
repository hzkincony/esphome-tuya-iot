import esphome.codegen as cg
from esphome import automation
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_ON_MESSAGE,
    CONF_ON_JSON_MESSAGE,
    CONF_TRIGGER_ID,
    CONF_TOPIC,
    CONF_QOS,
    CONF_PAYLOAD,
)

DEPENDENCIES = ["network", "time"]
AUTO_LOAD = ["json"]
# MULTI_CONF = True

tuya_iot_ns = cg.esphome_ns.namespace('tuya_iot')

TuyaComponent = tuya_iot_ns.class_('TuyaIotComponent', cg.PollingComponent)
TuyaIotMessageTrigger = tuya_iot_ns.class_(
    "TuyaIotMessageTrigger", automation.Trigger.template(cg.std_string), cg.Component
)
TuyaIotJsonMessageTrigger = tuya_iot_ns.class_(
    "TuyaIotJsonMessageTrigger", automation.Trigger.template(cg.JsonObjectConst)
)
TimeComponent = cg.esphome_ns.namespace('homeassistant').class_('HomeassistantTime')


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(TuyaComponent),
    cv.GenerateID("time_id"): cv.use_id(TimeComponent),
    cv.Required("product_id"): cv.string,
    cv.Required("device_id"): cv.string,
    cv.Required("device_secret"): cv.string,
    cv.Optional(CONF_ON_MESSAGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TuyaIotMessageTrigger),
            cv.Required(CONF_TOPIC): cv.subscribe_topic,
            cv.Optional(CONF_QOS, default=0): cv.mqtt_qos,
            cv.Optional(CONF_PAYLOAD): cv.string_strict,
        }
    ),
    cv.Optional(CONF_ON_JSON_MESSAGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                TuyaIotJsonMessageTrigger
            ),
            cv.Required(CONF_TOPIC): cv.subscribe_topic,
            cv.Optional(CONF_QOS, default=0): cv.mqtt_qos,
        }
    ),
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

    for conf in config.get(CONF_ON_MESSAGE, []):
        trig = cg.new_Pvariable(conf[CONF_TRIGGER_ID], conf[CONF_TOPIC])
        cg.add(trig.set_qos(conf[CONF_QOS]))
        if CONF_PAYLOAD in conf:
            cg.add(trig.set_payload(conf[CONF_PAYLOAD]))
        yield cg.register_component(trig, conf)
        yield automation.build_automation(trig, [(cg.std_string, "x")], conf)
    for conf in config.get(CONF_ON_JSON_MESSAGE, []):
        trig = cg.new_Pvariable(conf[CONF_TRIGGER_ID], conf[CONF_TOPIC], conf[CONF_QOS])
        yield automation.build_automation(trig, [(cg.JsonObjectConst, "x")], conf)
