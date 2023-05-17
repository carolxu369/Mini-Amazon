FROM ubuntu:20.04
RUN mkdir -p /Server/src
ADD . /Server/src
WORKDIR /Server/src
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get -yq install libboost-all-dev
RUN apt-get -yq install protobuf-compiler
RUN apt-get -yq install libpqxx-dev
RUN apt-get -yq install python3.8
RUN apt-get -yq install pip
RUN pip install SQLAlchemy
RUN pip install psycopg2-binary