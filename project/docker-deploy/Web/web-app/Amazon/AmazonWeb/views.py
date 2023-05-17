from django.shortcuts import render, redirect
from django.core.exceptions import ValidationError
from django.contrib import messages
from django.core.validators import validate_email
from . import models, form
from django.core.mail import BadHeaderError, send_mail
from django.db.models import Q
import json

# Create your views here.
from django.http import HttpResponse, HttpResponseRedirect

def welcome(request):
    # set up product database
    tender = models.Product.objects.filter(description="Chicken Tenders")
    fries = models.Product.objects.filter(description="Fries")
    coke = models.Product.objects.filter(description="Coke")
    umbrella = models.Product.objects.filter(description="Umbrella")
    sunglasses = models.Product.objects.filter(description="Sunglasses")
    hat = models.Product.objects.filter(description="Hat")

    if not tender:
        models.Product.objects.create(description="Chicken Tenders", price=8.89)
    if not fries:
        models.Product.objects.create(description="Fries", price=3.79)
    if not coke:
        models.Product.objects.create(description="Coke", price=2.95)
    if not umbrella:
        models.Product.objects.create(description="Umbrella", price=10.99)
    if not sunglasses:
        models.Product.objects.create(description="Sunglasses", price=20.99)
    if not hat:
        models.Product.objects.create(description="Hat", price=30.99)

    return render(request, 'welcome.html')

def register(request):
    if request.method == 'POST':
        user_name = request.POST.get('name')
        pwd = request.POST.get('pwd')
        re_pwd = request.POST.get('re_pwd')

        old_username = models.User.objects.filter(user_name=user_name)
        if old_username:
            messages.info(request, 'Username has already exist. Please log in.')
            return render(request, 'register.html')
        if not user_name:
            messages.info(request, 'Please enter username')
            return render(request, 'register.html')
        if pwd != re_pwd:
            messages.info(request, 'Password does not match!')
            return render(request, 'register.html')
        # create object
        try:
            models.User.objects.create(
                user_name=user_name, user_password=pwd)
            print('create a user')
        except ValidationError:
            messages.info(request, 'Username has already exist. Please log in.')
            return render(request, 'register.html')

        return render(request, 'login.html', locals())
    return render(request, 'register.html')

def login(request):
    if request.method == 'POST':
        user_name = request.POST['name']
        pwd = request.POST['pwd']
        try:
            find_user = models.User.objects.get(user_name=user_name)
        except Exception:
            messages.info(request, 'Username does not exist. Please try again.')
            return render(request, 'login.html')

        if pwd != find_user.user_password:
            messages.info(request, 'Incorrect password. Please try again.')
            return render(request, 'login.html')
        # save session
        request.session['user_name'] = user_name
        request.session['user_id'] = find_user.user_id

        # change
        return render(request, 'homepage.html', locals())

    return render(request, 'login.html')


def check_login(f):
    def wrap(request, *args, **kwargs):
        if not request.session.get('user_id'):
            return render(request, 'login.html')
        return f(request, *args, **kwargs)
    return wrap

# view function for log out
def logout(request):
    if ('user_name' in request.session):
        del request.session['user_name']
    if ('user_id' in request.session):
        del request.session['user_id']

    return HttpResponseRedirect('/welcome')

@check_login
def homepage(request):
    if request.method == 'POST':
        search = request.POST.get('search')
        if search.lower() == 'chicken tenders':
            return redirect('/buy_tenders')
        elif search.lower() == 'fries':
            return redirect('/buy_fries')
        elif search.lower() == 'coke':
            return redirect('/buy_coke')
        elif search.lower() == 'umbrella':
            return redirect('/buy_umbrella')
        elif search.lower() == 'sunglasses':
            return redirect('/buy_sunglasses')
        elif search.lower() == 'hat':
            return redirect('/buy_hat')
        else:
            messages.info(request, 'You need to enter the full product name as below, case insensitive.')
            return render(request, 'homepage.html')

    return render(request, 'homepage.html')

@check_login
def profile(request):
    if request.method == 'POST':
        user_name = request.POST.get('name')
        pwd = request.POST.get('pwd')
        re_pwd = request.POST.get('re_pwd')
        user_x = request.POST.get('user_x')
        user_y = request.POST.get('user_y')
        user_ups = request.POST.get('user_ups')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.") 
        
        if user_name:
            if user.user_name != user_name:
                old_username = models.User.objects.filter(user_name=user_name)
                if old_username:
                    messages.info(request, 'Username has already exist. Please enter another username.')
                    return render(request, 'profile.html')
                else:
                    user.user_name = user_name

        if pwd:
            if pwd != re_pwd:
                messages.info(request, 'Password does not match!')
                return render(request, 'profile.html')
            else:
                user.user_password = pwd

        if user_x:
            if (user_x.isdigit()):
                user.default_addr_x = user_x
            else:
                messages.info(request, 'The user default address x should be an integer. Please enter again.')
                return render(request, 'profile.html')
            
        if user_y:
            if (user_y.isdigit()):
                user.default_addr_y = user_y
            else:
                messages.info(request, 'The user default address y should be an integer. Please enter again.')
                return render(request, 'profile.html')
            
        if user_ups:
            user.default_ups_account = user_ups

        user.save()
        return render(request, 'homepage.html', locals())
    
    return render(request, 'profile.html')

@check_login
def buy_tenders(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Chicken Tenders")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_tenders.html')

@check_login
def buy_fries(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Fries")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_fries.html')

@check_login
def buy_coke(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Coke")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_coke.html')

@check_login
def buy_umbrella(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Umbrella")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_umbrella.html')

@check_login
def buy_sunglasses(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Sunglasses")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_sunglasses.html')

@check_login
def buy_hat(request):
    if request.method == 'POST':
        quantity = request.POST.get('quantity')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        try:
            product = models.Product.objects.get(description = "Hat")
        except Exception:
            return HttpResponse("The product doesn't exist.")
        
        # add prduct to the shopping table
        product_exist = models.Shopping.objects.filter(user_id=user.user_id).filter(product_id=product.product_id)

        if product_exist:
            curr_product = models.Shopping.objects.get(user_id=user.user_id, product_id=product.product_id)
            curr_product.product_num = quantity
            curr_product.save()
        else:
            models.Shopping.objects.create(product_id=product, product_num=quantity, user_id=user)

        return redirect('/shopping_cart')
        # return render(request, 'shopping_cart.html', locals())
    
    return render(request, 'buy_hat.html')

@check_login
def shopping_cart(request):
    if request.method == 'GET':
        products = models.Shopping.objects.filter(user_id=request.session['user_id'])
        
        if products:
            is_exist = 1
        else:
            is_exist = 0

        return render(request, 'shopping_cart.html', locals())
    
    if request.method == "POST":
        if "remove" in request.POST:
            product_id = request.POST["remove"]
            if product_id:
                models.Shopping.objects.filter(product_id=product_id, user_id=request.session['user_id']).delete()
            return redirect("/shopping_cart")
        elif "checkout" in request.POST:
            return redirect("/checkout")

    return render(request, 'shopping_cart.html', locals())

@check_login
def checkout(request):
    if request.method == 'POST':
        dest_x = request.POST.get('dest_x')
        dest_y = request.POST.get('dest_y')
        order_ups = request.POST.get('order_ups')
        card_num = request.POST.get('card_num')

        try:
            user = models.User.objects.get(user_id = request.session['user_id'])
        except Exception:
            return HttpResponse("The user doesn't exist.")
        
        if not dest_x:
            default_x = user.default_addr_x
            if not default_x:
                messages.info(request, 'You do not have a default address x and not enter a destination. Please enter destination address x.')
                return render(request, 'checkout.html')
            else:
                dest_x = default_x

        if not dest_y:
            default_y = user.default_addr_y
            if not default_y:
                messages.info(request, 'You do not have a default address y and not enter a destination. Please enter destination address y.')
                return render(request, 'checkout.html')
            else:
                dest_y = default_y

        if not order_ups:
            default_ups = user.default_ups_account
            if default_ups:
                order_ups = default_ups

        # create an order
        curr_order = models.Order.objects.create(user_id=user, dest_addr_x=dest_x, dest_addr_y=dest_y, ups_account=order_ups, order_status='open')

        # update productlist
        products = models.Shopping.objects.filter(user_id = request.session['user_id'])
        for product in products:
            models.ProductList.objects.create(product_id=product.product_id, product_num=product.product_num, order_id=curr_order)

        # clear shopping table once checkout
        models.Shopping.objects.filter(user_id=request.session['user_id']).delete()

        return render(request, 'homepage.html', locals())
    
    return render(request, 'checkout.html')

@check_login
def order(request):
    if request.method == 'GET':
        user = models.User.objects.get(user_id = request.session['user_id'])
        orders = user.orders.all()

        # update order status
        if orders:
            for order in orders:
                status_flag = 1 # 1 indicate complete
                packages = order.packages.all()
                if packages:
                    for package in packages:
                        if package.status != 'delivered':
                            status_flag = 0
                else:
                    continue

                if status_flag == 1:
                    order.order_status = 'complete'

                order.save()

        if orders:
            for order in orders:
                for product in order.lists.all():
                    package_flag = 0
                    curr_des = product.product_id.description
                    for package in order.packages.all():
                        if package.description == curr_des:
                            if models.ProductStatus.objects.filter(order_id=order, description=curr_des):
                                curr_product = models.ProductStatus.objects.get(order_id=order, description=curr_des)
                                curr_product.status = package.status
                                curr_product.tracking_number = package.tracking_number
                                curr_product.save()
                            else:
                                models.ProductStatus.objects.create(order_id=order, description=curr_des, status=package.status, tracking_number=package.tracking_number)
                            package_flag = 1
                            break
                    if package_flag == 0:
                        if not models.ProductStatus.objects.filter(order_id=order, description=curr_des):
                            models.ProductStatus.objects.create(order_id=order, description=curr_des)

        # get updated orders
        new_orders = user.orders.all()
        is_exist = 0
        if new_orders:
            is_exist = 1

        context = {
            'is_exist': is_exist,
            'orders': new_orders
        }

        return render(request, 'order.html', context)

    return render(request, 'order.html', locals())


