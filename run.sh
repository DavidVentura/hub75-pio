make && openocd -f ../myfile.cfg -f target/rp2040.cfg -c "program main.elf reset exit"
