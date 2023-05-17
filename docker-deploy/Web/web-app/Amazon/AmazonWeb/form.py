from django import forms
from django.forms import widgets
from django.contrib import messages
from django.core.validators import MaxValueValidator, MinValueValidator


class DateTimeLocalInput(forms.DateTimeInput):
    input_type = "datetime-local"


class DateTimeLocalField(forms.DateTimeField):
    input_formats = [
        "%Y-%m-%dT%H:%M:%S",
        "%Y-%m-%dT%H:%M:%S.%f",
        "%Y-%m-%dT%H:%M"
    ]
    widget = DateTimeLocalInput(format="%Y-%m-%dT%H:%M")

class start_ride_form(forms.Form):
    arrival_time = DateTimeLocalField(error_messages={"required": "Please enter arrival time"})
    passenger_number = forms.IntegerField(validators=[MinValueValidator(1)], error_messages={"required": "Please enter passenger number >= 1"})
    des = forms.CharField(error_messages={"required": "Please enter destination"})
    vehicle_type = forms.CharField(error_messages={"required": "Please enter vehicle type"})
    is_share = forms.ChoiceField(choices=[("True", True), ("False", False)],error_messages={"required": "Please choose share or not"})

class join_ride_form(forms.Form):
    des = forms.CharField(error_messages={"required": "Please enter destination"})
    ealiest_time = DateTimeLocalField(error_messages={"required": "Please enter arrival time"})
    latest_time = DateTimeLocalField(error_messages={"required": "Please enter arrival time"})
    passenger_number = forms.IntegerField(validators=[MinValueValidator(1)], error_messages={"required": "Please enter passenger number >= 1"})

class reg_driver_form(forms.Form):
    driver_name = forms.CharField(error_messages = {"required": "This field cannot be empty: Please enter your name"})
    vehicle_type = forms.CharField(error_messages = {"required": "This field cannot be empty: Please enter your vehicle type"})
    lpn = forms.CharField(error_messages = {"required": "This field cannot be empty: Please enter your License Plate Number"})
    max_passenger = forms.IntegerField(validators=[MinValueValidator(1)], error_messages={"required": "This field cannot be empty: The maximum number of passengers should be >= 1"})