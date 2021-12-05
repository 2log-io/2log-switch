FROM ubuntu:latest
RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y gcc git wget make libncurses-dev flex bison gperf python python3-serial python3-pip

RUN mkdir -p /esp
WORKDIR /esp
#RTOS SDK v. 3.3
RUN wget https://dl.espressif.com/dl/xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz
RUN tar -xzf xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz
ENV PATH="$PATH:$HOME/esp/xtensa-lx106-elf/bin"
RUN git clone https://github.com/espressif/ESP8266_RTOS_SDK.git
WORKDIR /esp/ESP8266_RTOS_SDK
RUN git checkout 1f5bed1c5ed4dbba8d9911cbab5b41a19f971d37
RUN git submodule init && git submodule update
ENV IDF_PATH=/esp/ESP8266_RTOS_SDK
RUN pip3 install --user -r /esp/ESP8266_RTOS_SDK/requirements.txt
RUN rm /usr/bin/python && ln -s /usr/bin/python3 /usr/bin/python
WORKDIR /src
