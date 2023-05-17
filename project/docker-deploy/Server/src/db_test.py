from sqlalchemy import create_engine, ForeignKey, Column, Integer, String, TIMESTAMP, Float, Sequence, MetaData
from sqlalchemy.orm import relationship
from sqlalchemy.engine import URL
from sqlalchemy.orm import sessionmaker, scoped_session, declarative_base
from sqlalchemy.exc import DBAPIError
from sqlalchemy.sql import func
from sqlalchemy.pool import NullPool

Base = declarative_base()

class Product(Base):
    __tablename__ = 'Product'
    
    product_id = Column(Integer, Sequence('product_id_seq'), primary_key=True)
    description = Column(String, unique=True)
    price = Column(Float, nullable=False)
    
    def __repr__(self):
        return "<Product(product_id='%s', description='%s', price='%s')>" % (self.product_id, self.description, self.price)

class User(Base):
    __tablename__ = 'User'
    
    user_id = Column(Integer, Sequence('user_id_seq'), primary_key=True)
    user_name = Column(String, unique=True)
    
    def __repr__(self):
        return "<User(user_id='%s', user_name='%s')>" % (self.user_id, self.user_name)

class Warehouse(Base):
    __tablename__ = 'Warehouse'
    
    warehouse_id = Column(Integer, primary_key=True)
    ware_addr_x = Column(Integer, nullable=False)
    ware_addr_y = Column(Integer, nullable=False)
    truck_status = Column(String, nullable=False)
    truck_id = Column(Integer, nullable=True)
    
    def __repr__(self):
        return "<Warehouse(warehouse_id='%s', ware_addr_x='%s', ware_addr_y='%s')>" % (self.warehouse_id, self.ware_addr_x, self.ware_addr_y)

class Order(Base):
    __tablename__ = 'Order'
    
    order_id = Column(Integer, Sequence('order_id_seq'), primary_key=True)
    user_id = Column(Integer, ForeignKey(User.user_id))
    dest_addr_x = Column(Integer, nullable=False)
    dest_addr_y = Column(Integer, nullable=False)
    ups_account = Column(String, nullable=True)
    order_time = Column(TIMESTAMP(timezone=True), server_default=func.now())
    order_status = Column(String, nullable=False)
    
    def __repr__(self):
        return "<Order(order_id='%s', user_id='%s', order_time'%s', order_status='%s')>" % (self.order_id, self.user_id, self.order_time, self.order_status)

class ProductList(Base):
    __tablename__ = 'ProductList'
    
    serial_id = Column(Integer, Sequence('product_list_id_seq'), primary_key=True)
    product_id = Column(Integer, ForeignKey(Product.product_id))
    product_num = Column(Integer, nullable=False)
    order_id = Column(Integer, ForeignKey(Order.order_id))
    warehouse_id = Column(Integer, ForeignKey(Warehouse.warehouse_id), nullable=True)

    def __repr__(self):
        return "<ProductList(serial_id='%s', product_id='%s', product_num='%s', order_id='%s', warehouse_id='%s')>" % (self.serial_id, self.product_id, self.product_num, self.order_id, self.warehouse_id)
    
class Package(Base):
    __tablename__ = 'Package'
    
    package_id = Column(Integer, primary_key=True)
    order_id = Column(Integer, ForeignKey(Order.order_id))
    warehouse_id = Column(Integer, ForeignKey(Warehouse.warehouse_id))
    status = Column(String, nullable=False)
    description = Column(String)
    tracking_number = Column(Integer, nullable=True)
    truck_id = Column(Integer, nullable=True)
    
    def __repr__(self):
        return "<Package(package_id='%s', order_id='%s', warehouse_id='%s', status='%s', description='%s', tracking_number='%s', truck_id='%s')>" \
                        % (self.package_id, self.order_id, self.warehouse_id, self.status, self.description, self.tracking_number, self.truck_id)

def try_add(tuple):
    session = get_session()
    session.add(tuple)
    try:
        session.commit()
    except DBAPIError as ex:
        session.rollback()
        print(str(ex))

def add_product(description, price):
    new_product = Product(description=description, price=price)
    try_add(new_product)

def add_user(name):
    new_user = User(user_name=name)
    try_add(new_user)
    
def add_warehouse(id, x, y):
    new_warehouse = Warehouse(warehouse_id=id, ware_addr_x=x, ware_addr_y=y, truck_status="not_requested")
    try_add(new_warehouse)

def add_order(user_id, x, y, order_status):
    new_order = Order(user_id=user_id, dest_addr_x=x, dest_addr_y=y, order_status=order_status)
    try_add(new_order)

def add_product_list(product_id, num, order_id, warehouse_id=None):
    new_productlist = ProductList(product_id=product_id, product_num=num, order_id=order_id, warehouse_id=warehouse_id)
    try_add(new_productlist)
    
def add_package(order_id, warehouse_id, status, description, tracking_number, truck_id):
    new_package = Package(order_id=order_id, warehouse_id=warehouse_id, status=status, description=description, tracking_number=tracking_number, truck_id=truck_id)
    try_add(new_package)

def init(drop):
    url = URL.create(
        drivername="postgresql",
        username="postgres",
        host="127.0.0.1",
        # host="db",
        database="ece568_project",
        password="passw0rd"

    )
    engine = create_engine(url, poolclass=NullPool)
    # global session
    # session = scoped_session(sessionmaker(bind=engine))
    if drop is True:
        # use the code below to drop all table before initialization
        # metadata = MetaData()
        # metadata.reflect(bind=engine)
        Base.metadata.drop_all(bind=engine)
    Base.metadata.create_all(engine)

def get_session():
    url = URL.create(
        drivername="postgresql",
        username="postgres",
        host="127.0.0.1",
        # host="db",
        database="ece568_project",
        password="passw0rd"
    )
    engine = create_engine(url, poolclass=NullPool)
    global session
    session = scoped_session(sessionmaker(bind=engine))
    return session


if __name__ == "__main__":
    init(True)
    add_user("user1")
    add_product("product1", 10.88)
    add_product("product2", 20.99)
    add_order(1, 5, 6, "open")
    add_product_list(1, 2, 1)
    add_product_list(2, 3, 1)
    
    # session = get_session()
    # packages = session.query(Package).filter(Package.order_id == 1)
    # for package in packages:
    #     warehouse_id = package.warehouse_id
    #     warehouse = session.query(Warehouse).filter(Warehouse.warehouse_id == warehouse_id).first()
    #     print(warehouse)
    #     warehouse.truck_status = "arrived"
    #     warehouse.truck_id = 1
    # session.commit()