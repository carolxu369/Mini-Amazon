# Generated by Django 4.1.5 on 2023-04-24 19:01

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('AmazonWeb', '0005_alter_order_order_status_alter_package_status_and_more'),
    ]

    operations = [
        migrations.CreateModel(
            name='ProductStatus',
            fields=[
                ('serial_id', models.AutoField(primary_key=True, serialize=False)),
                ('description', models.CharField(default='', max_length=128)),
                ('status', models.CharField(blank=True, choices=[('packing', 'packing'), ('packed', 'packed'), ('loading', 'loading'), ('loaded', 'loaded'), ('delivering', 'delivering'), ('delivered', 'delivered')], max_length=128, null=True)),
                ('tracking_number', models.CharField(blank=True, max_length=128, null=True)),
                ('order_id', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='products', to='AmazonWeb.order')),
            ],
        ),
    ]