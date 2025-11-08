FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        cmake \
        build-essential \
        libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /proj

COPY . /proj 

RUN chmod +x /proj/run.sh

CMD ["./run.sh"]