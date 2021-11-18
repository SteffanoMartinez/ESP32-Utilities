# Why should I use these libraries?

These libraries are the results of working with the ESP32 family of MCUs for 3+ years. These libraries are great for writting performant applications, fast. 

If you've wrote code using the Arduino framework, these libraries are here to help you take it to the next step. Although I love the ESP-IDF for the ESP32, it's only made for the ESP32 MCU, and I would like to use other chips for different applications, in the future. 

These libraries are very easy to port to other architectures like the iMXRT processor in the Teensy boards, and I have plans of doing so in the future.

# Getting Started

The **simple** examples folder has enumerated examples that cover each of the libraries in great detail. I recommend you start there & read all the comments & instructions. I've linked several resources where necessary, and I highly encourage you to read these articles before diving into using the libraries. 

## Hardware needed

- ESP32 Development board
- MCP2518FD CAN 2.0B/FD Controller
- AD7689 16-bit ASR Analog to Digital (ADC) converter
- MPU9250 Internal Motion Unit (IMU)
- BME688 Environmental sensor
- RV-3028 Real Time Clock (RTC)
- uSD Card

## Error Handling

These libraries end up being called by a higher abstraction layers, or in some cases many. In order to be able to pass any errors to higher abstraction layers, all functions that **can** return a known error, are of the type **ESP_ERROR**, en example is given below:

# Why should I use these libraries?

These libraries are the results of working with the ESP32 family of MCUs for 3+ years. These libraries are great for writting performant applications, fast. 

If you've wrote code using the Arduino framework, these libraries are here to help you take it to the next step. Although I love the ESP-IDF for the ESP32, it's only made for the ESP32 MCU, and I would like to use other chips for different applications.

# Getting Started

The **simple** examples folder has enumerated examples that cover each of the libraries in great detail. I recommend you start there & read all the comments & instructions. I've linked several resources where necessary, and I highly encourage you to read these articles before diving into using the libraries. 

## Hardware needed

- ESP32 Development board
- MCP2518FD CAN 2.0B/FD Controller
- AD7689 16-bit ASR Analog to Digital (ADC) converter
- MPU9250 Internal Motion Unit (IMU)
- BME688 Environmental sensor
- RV-3028 Real Time Clock (RTC)
- uSD Card

## Error Handling

These libraries end up being called by a higher abstraction layers, or in some cases many. In order to be able to pass any errors to higher abstraction layers, all functions that **can** return a known error, are of the type **ESP_ERROR**.

For example, let's say you call the following method to make a new directory without initializing the card:

``` C++
ESP_ERROR EMMC_Memory::makeDirectory(const char *path)
{
    ESP_ERROR err;
    String temp_message;

    if (emmc_initialized)
    {
        if (file_system->mkdir(path))
        {
            temp_message += "Directory \"";
            temp_message += path;
            temp_message += "\"";
            temp_message += " created succesfully";
        }
        else
        {
            err.on_error = true;
            temp_message += "Error creating directory";
        }
    }
    else
    {
        err.on_error = true;
        temp_message += "External storage is not inititalized";
    }

    err.debug_message = temp_message;
    return err;
}
```

On your application layer, this would look something like the following:

``` C++
void loop(){
  
  ESP_ERROR make_directory = emmc.makeDirectory("/test");
  
  if(make_directory.on_error)
    abort(make_directory.debug_message); // Or any other debugging function call of your choice

}
```

And you will see the following output in your serial terminal

```
[            51 mS] [+   11 mS] [1] [INF] [TER] - External storage is not inititalized

Stopping program. Restart
```

Along with these messages, Espressif's debug output will also be sent to the terminal on **abort**. This will help you find bugs & problems in your apps quickly & efficiently.