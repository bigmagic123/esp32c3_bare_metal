# esp32c3_bare_metal
## how to build
*1.modify cross.txt*
set your gcc pass
you can download gcc from 

```
https://xpack.github.io/riscv-none-embed-gcc/
```

*2.build hello_world*

cd hello_world
```
meson setup _build --cross-file cross.txt
```

*3.build*

```
cd _build
ninja
```

## how to download
*windows*

first you need install python3

```
pip install esptool
```

then

```
esptool.py --port COM3 --baud 921600 write_flash 0x0000 main.bin
python -m serial.tools.miniterm COM3 115200
```
exit you can
```
ctrl + ]
```