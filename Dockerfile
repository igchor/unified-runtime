# Start with: sudo docker run -d -p $PORT:22 --security-opt seccomp:unconfined -e http_proxy=$HTTP_PROXY -e https_proxy=$HTTPS_PROXY -v /home/igor/UR:/home/igor/UR --device=/dev/dri/renderD128 -w /home/igor/UR -it ur-python /bin/bash
FROM ghcr.io/intel/llvm/ubuntu2204_intel_drivers

RUN apt-get update && apt-get install -y openssh-server gdb gdbserver pip python3

COPY third_party/requirements.txt requirements.txt
RUN pip install -r requirements.txt

RUN mkdir -p /var/run/sshd
RUN echo 'root:root' | chpasswd \
    && sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

CMD ["/usr/sbin/sshd", "-D"]

EXPOSE 22
