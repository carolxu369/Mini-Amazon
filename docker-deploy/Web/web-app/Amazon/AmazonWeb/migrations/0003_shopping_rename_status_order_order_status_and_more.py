# Generated by Django 4.1.5 on 2023-04-23 13:39

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('AmazonWeb', '0002_package_description'),
    ]

    operations = [
        migrations.CreateModel(
            name='Shopping',
            fields=[
                ('serial_id', models.AutoField(primary_key=True, serialize=False)),
                ('product_num', models.PositiveIntegerField(default=1)),
                ('product_id', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='shopping', to='AmazonWeb.product')),
                ('user_id', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='shopping', to='AmazonWeb.user')),
            ],
        ),
        migrations.RenameField(
            model_name='order',
            old_name='status',
            new_name='order_status',
        ),
        migrations.RenameField(
            model_name='package',
            old_name='tracking_num',
            new_name='tracking_number',
        ),
        migrations.AddField(
            model_name='warehouse',
            name='truck_id',
            field=models.IntegerField(blank=True, null=True),
        ),
        migrations.AddField(
            model_name='warehouse',
            name='truck_status',
            field=models.IntegerField(choices=[(1, 'not_requested'), (2, 'requested'), (3, 'arrived')], default=1),
        ),
        migrations.AlterField(
            model_name='package',
            name='truck_id',
            field=models.IntegerField(blank=True, null=True),
        ),
        migrations.DeleteModel(
            name='Truck',
        ),
    ]
