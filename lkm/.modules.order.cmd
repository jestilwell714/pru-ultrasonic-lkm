cmd_/home/debian/301sensor_project/lkm/modules.order := {   echo /home/debian/301sensor_project/lkm/sensor_module.ko; :; } | awk '!x[$$0]++' - > /home/debian/301sensor_project/lkm/modules.order
