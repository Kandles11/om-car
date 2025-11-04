import cantools
from cantools.database.can import Message, Signal, Database, Node

db = Database()

dynamics_accel = Node(
    name="Dynamics Acceleration Pedal"
)

dynamics_output = Node(
    name="Dynamics Output"
)


pedal_position = Signal(
    name="Accelerator Pedal Position",
    length=8,
    start=0,
    minimum=-100,
    maximum=100,
    unit="%",
    comment="The position of the accelerator pedal",
)

accel_pedal_message = Message(
    name="Accelerator Pedal Message",
    frame_id=0x100,
    length=1,
    signals=[pedal_position],
)

db.messages.append(accel_pedal_message)

cantools.database.dump_file(db, 'omcar.dbc')

