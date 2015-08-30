sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash --debug 1 my_original_firmware_clone_R3.hex
sudo dfu-programmer atmega16u2 reset
