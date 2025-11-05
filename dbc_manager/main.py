from typing import Any
import cantools
from cantools.database.can import Message, Signal, Database, Node
from cantools.database.conversion import BaseConversion, LinearIntegerConversion, NamedSignalConversion


db = Database()

front_zonal = Node(
    name="front_zonal"
)

rear_zonal = Node(
    name="rear_zonal"
)

light_zonal = Node(
    name="light_zonal"
)

input_zonal = Node(
    name="input_zonal"
)

display = Node(
    name="display"
)

## ======================== FRONT ZONAL ========================
req_motor_power = Signal(
    name="Requested Motor Power",
    length=8,
    minimum=-100,
    maximum=100,
    start=0,
    is_signed=True,
    unit="%",
    receivers=[],
    comment="The requested motor power",
)

req_motor_power_msg = Message(
    name="Requested Motor Power Message",
    frame_id=0x101,
    length=1,
    signals=[req_motor_power],
    senders=["front_zonal"],
)

db.messages.append(req_motor_power_msg)

## ======================== REAR ZONAL ========================

wheel_speed = Signal(
    name="Wheel Speed",
    length=8,
    minimum=0,
    maximum=255,
    start=0,
    is_signed=False,
    unit="km/h",
    receivers=["display"],
    comment="The speed of the wheels",
)

odometer = Signal(
    name="Odometer",
    length=16,
    minimum=0,
    maximum=65535,
    start=8,
    is_signed=False,
    unit="km",
    receivers=["display"],
    comment="The odometer reading",
)

drive_stats = Message(
    name="Drive Statistics",
    frame_id=0x201,
    length=3,
    signals=[wheel_speed, odometer],
    senders=["rear_zonal"],
)

battery_voltage = Signal(
    name="Battery Voltage",
    length=16,
    minimum=0,
    maximum=30,
    start=0,
    is_signed=False,
    unit="V",
    conversion=LinearIntegerConversion.factory(scale=10, offset=0),
    receivers=["display"],
    comment="The voltage of the battery"
)

battery_voltage_msg = Message(
    name = "Battery Voltage",
    frame_id=0x202,
    length = 2,
    signals = [battery_voltage],
    senders = ["rear_zonal"],
)

db.messages.extend([battery_voltage_msg, drive_stats])

## ======================== INPUT ZONAL ========================

headlight_state = Signal(
    name="Headlight Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=0,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"off", 1:"on"}, is_float=False),
    receivers=["front_zonal", "display"],
    comment="Requested state of headlights"
)

left_blinker_state = Signal(
    name="Left Blinker Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=1,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"off", 1:"on"}, is_float=False),
    receivers=["light_zonal", "display"],
    comment="Requested state of left blinker"
)

right_blinker_state = Signal(
    name="Right Blinker Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=2,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"off", 1:"on"}, is_float=False),
    receivers=["light_zonal", "display"],
    comment="Requested state of right blinker"
)

hazard_lights_state = Signal(
    name="Hazard Lights Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=3,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"off", 1:"on"}, is_float=False),
    receivers=["light_zonal", "display"],
    comment="Requested state of hazard lights"
)

light_control_msg = Message(
    name = "Light Controls",
    frame_id=0x301,
    length = 1,
    signals = [headlight_state, left_blinker_state, right_blinker_state, hazard_lights_state],
    senders = ["input_zonal"],
)

horn_state = Signal(
    name="Horn Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=0,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"off", 1:"on"}, is_float=False),
    receivers=[],
    comment="Requested state of the horn"
)

horn_control_msg = Message(
    name = "Horn Control",
    frame_id=0x302,
    length = 1,
    signals = [horn_state],
    senders = ["input_zonal"],
)

gear_control_state = Signal(
    name="Gear Control Switch State",
    length=1,
    minimum=0,
    maximum=1,
    start=0,
    is_signed=False,
    unit=None,
    conversion=NamedSignalConversion.factory(scale=1, offset=0, choices={0:"forward", 1:"reverse"}, is_float=False),
    receivers=["display"],
    comment="Requested state of the gear"
)

gear_select_msg = Message(
    name = "Gear Select",
    frame_id=0x303,
    length = 1,
    signals = [gear_control_state],
    senders = ["input_zonal"],
)

db.messages.extend([gear_select_msg, horn_control_msg, light_control_msg])


cantools.database.dump_file(db, 'output/omcar.dbc')

