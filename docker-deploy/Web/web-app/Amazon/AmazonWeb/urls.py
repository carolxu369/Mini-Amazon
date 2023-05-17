from django.urls import path

from . import views

urlpatterns = [
    path('welcome', views.welcome),
    path('register', views.register),
    path('login',views.login),
    path('homepage', views.homepage),
    path('profile', views.profile),
    path('buy_tenders', views.buy_tenders),
    path('shopping_cart', views.shopping_cart),
    path('checkout', views.checkout),
    path('order', views.order),
    path('buy_fries', views.buy_fries),
    path('buy_coke', views.buy_coke),
    path('buy_umbrella', views.buy_umbrella),
    path('buy_sunglasses', views.buy_sunglasses),
    path('buy_hat', views.buy_hat)
]