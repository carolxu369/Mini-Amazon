FROM python:3
ENV PYTHONUNBUFFERED 1
RUN mkdir -p /Web/web-app
WORKDIR /Web/web-app
ADD requirements.txt ./
RUN pip install -r requirements.txt
ADD . /