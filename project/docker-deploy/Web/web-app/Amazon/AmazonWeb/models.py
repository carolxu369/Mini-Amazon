from django.db import models
from django.utils.timezone import now

# Create your models here.

class Product(models.Model):
    # primary id
    product_id = models.AutoField(primary_key=True)
    # unique but not primary
    description = models.CharField(max_length=128, null=False, blank=False, unique=True)
    price = models.FloatField(default=0.01, null=False, blank=False)

class User(models.Model):
    # primary id
    user_id = models.AutoField(primary_key=True)
    # unique for login
    user_name = models.CharField(max_length=128, null=False, blank=False, unique=True)
    user_password = models.CharField(max_length=128, null=False, blank=False) 

    # following fields are optional
    default_addr_x = models.IntegerField(default=0, null=True, blank=True)
    default_addr_y = models.IntegerField(default=0, null=True, blank=True)
    default_ups_account = models.CharField(max_length=128, null=True, blank=True)

    def __str__(self):
        return self.user_name

class Warehouse(models.Model):
    # primary_id
    warehouse_id = models.AutoField(primary_key=True)
    ware_addr_x = models.IntegerField(default=0, null=False, blank=False)
    ware_addr_y = models.IntegerField(default=0, null=False, blank=False)
    status_choice = (('not_requested', 'not_requested'), ('requested', 'requested'), ('arrived', 'arrived'))
    truck_status = models.CharField(max_length=128, choices=status_choice, null=False, default='not_requested')
    truck_id = models.IntegerField(null=True, blank=True)

# class Truck(models.Model):
#     # primary_id
#     truck_id = models.AutoField(primary_key=True)
#     warehouse_id = models.ForeignKey(Warehouse, related_name='trucks', on_delete=models.SET_NULL, null=True)

class Order(models.Model):
    # primary_id
    order_id = models.AutoField(primary_key=True)
    user_id = models.ForeignKey(User, related_name='orders', on_delete=models.CASCADE)
    dest_addr_x = models.IntegerField(default=0, null=False, blank=False)
    dest_addr_y = models.IntegerField(default=0, null=False, blank=False)
    ups_account = models.CharField(max_length=128, null=True, blank=True)
    order_time = models.DateTimeField(default=now)
    status_choice = (('open', 'open'), ('processing', 'processing'), ('complete', 'complete'))
    order_status = models.CharField(max_length=128, choices=status_choice, null=False, default='open')

    def __str__(self):
        return str(self.order_id)

class ProductList(models.Model):
    # primary_id
    serial_id = models.AutoField(primary_key=True)
    product_num = models.PositiveIntegerField(default=1, null=False, blank=False)
    product_id = models.ForeignKey(Product, related_name='lists', on_delete=models.CASCADE)
    order_id = models.ForeignKey(Order, related_name='lists', on_delete=models.CASCADE)
    warehouse_id = models.ForeignKey(Warehouse, related_name='lists', on_delete=models.CASCADE, null=True, blank=True)

class Package(models.Model):
    # primary_id
    package_id = models.AutoField(primary_key=True)
    order_id = models.ForeignKey(Order, related_name='packages', on_delete=models.CASCADE)
    warehouse_id = models.ForeignKey(Warehouse, related_name='packages', on_delete=models.CASCADE)
    description = models.CharField(max_length=128, null=False, blank=False, default="")
    status_choice = (('packing', 'packing'), ('packed', 'packed'), ('loading', 'loading'), ('loaded', 'loaded'), ('delivering', 'delivering'), ('delivered', 'delivered'))
    status = models.CharField(max_length=128, choices=status_choice, null=False, default='packing')
    # return by the ups side
    tracking_number = models.CharField(max_length=128, null=True, blank=True)
    truck_id = models.IntegerField(null=True, blank=True)

# for frond-end shopping cart use only
class Shopping(models.Model):
    # primary_id
    serial_id = models.AutoField(primary_key=True)
    product_id = models.ForeignKey(Product, related_name='shopping', on_delete=models.CASCADE)
    product_num = models.PositiveIntegerField(default=1, null=False, blank=False)
    user_id = models.ForeignKey(User, related_name='shopping', on_delete=models.CASCADE)

class ProductStatus(models.Model):
    # primary_id
    serial_id = models.AutoField(primary_key=True)
    order_id = models.ForeignKey(Order, related_name='products', on_delete=models.CASCADE)
    description = models.CharField(max_length=128, null=False, blank=False, default="")
    status_choice = (('packing', 'packing'), ('packed', 'packed'), ('loading', 'loading'), ('loaded', 'loaded'), ('delivering', 'delivering'), ('delivered', 'delivered'))
    status = models.CharField(max_length=128, choices=status_choice, null=True, blank=True)
    tracking_number = models.CharField(max_length=128, null=True, blank=True)
