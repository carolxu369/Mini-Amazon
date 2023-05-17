from sqlalchemy import MetaData, Table, create_engine
from sqlalchemy.engine import URL
from sqlalchemy.orm import  sessionmaker, scoped_session, declarative_base

if __name__ == "__main__":
    url = URL.create(
        drivername="postgresql",
        username="postgres",
        host="db",
        database="amazon",
        password="passw0rd"
    )
    engine = create_engine(url)
    session = scoped_session(sessionmaker(bind=engine))
    Base = declarative_base()
    Base.metadata.reflect(engine)
    meta = MetaData()
    productstatus = Table('AmazonWeb_productstatus', meta, autoload_with=engine)
    order = Table('AmazonWeb_order', meta, autoload_with=engine)
    package = Table('AmazonWeb_package', meta, autoload_with=engine)
    product_list = Table('AmazonWeb_productlist', meta, autoload_with=engine)
    warehouse = Table('AmazonWeb_warehouse', meta, autoload_with=engine)
    session.query(productstatus).delete()
    session.query(order).delete()
    session.query(package).delete()
    session.query(product_list).delete()
    session.query(warehouse).delete()
    session.commit()