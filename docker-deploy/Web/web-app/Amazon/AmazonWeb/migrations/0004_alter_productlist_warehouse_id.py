# Generated by Django 4.1.5 on 2023-04-23 21:09

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('AmazonWeb', '0003_shopping_rename_status_order_order_status_and_more'),
    ]

    operations = [
        migrations.AlterField(
            model_name='productlist',
            name='warehouse_id',
            field=models.ForeignKey(blank=True, null=True, on_delete=django.db.models.deletion.CASCADE, related_name='lists', to='AmazonWeb.warehouse'),
        ),
    ]
