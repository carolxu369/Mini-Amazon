# erss-project-yz674-jx139
## Usage
This project has been deployed with Docker. To launch the project, after launching the world simulator, run
```bash
sudo docker-compose up
```
Both Django Web application and Back-end server should be launched simultaneously. 

Below shows the process of individually launching Front-end and Back-end applications, which the docker scripts has already done for you.

Front-end: Django Web

To set up database:
```bash
/web-app/Amazon$ python3 manage.py makemigrations
/web-app/Amazon$ python3 manage.py migrate
```

To run front-end:
```bash
/web-app/Amazon$ python3 manage.py runserver 0:8000
```

Back-end: C++ Server

Before running server, first clean up existing database:
```bash
Server/src$ python3 db_cleanup.py
```

To run server:
```bash
Server/src$ ./main
```
