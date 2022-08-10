# DPP example

# Overview
Dining Philosophers Problem (DPP) example with multiple
active objects.


# Building and Running (Linux)

Open a terminal in the directory of this example (`qpcpp/examples/zephry/dpp`).

Depending on your Zephyr installation, you might need to shell the script
to configure Zephyr environment:

```
source ~/zephyrproject/zephyr/zephyr-env.sh
```

To build the example, type:
```
west build -b <board>
```
where `<board>` is one of the
[boards supported by Zephyr](https://docs.zephyrproject.org/latest/boards/index.html).
The `dpp` example has been tested with the following boards:

```
nucleo_l152re
nucleo_h743zi
```

To flush the board, type:
```
west flush
```

# Sample Output
The application also produces ASCII output to the serial terminal
(if suppoted by the board):

```
*** Booting Zephyr OS build v2.6.0-rc2-88-g3d39f72a88b3  ***
Philo[4]->thinking
Philo[3]->eating  
Philo[1]->thinking
Philo[0]->eating  
Philo[4]->hungry  
Philo[3]->thinking
Philo[2]->eating  
Philo[1]->hungry  
Philo[0]->thinking
Philo[4]->eating  
Philo[3]->hungry  
Philo[0]->hungry  
Philo[4]->thinking
Philo[0]->eating  
Philo[2]->thinking
Philo[3]->eating  
Philo[4]->hungry  
Philo[2]->hungry  
Philo[3]->thinking
Philo[2]->eating  
Philo[0]->thinking
```

