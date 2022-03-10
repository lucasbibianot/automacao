import pika

cloud_url = 'amqps://nqfzvqjw:whkIlJKBEn-syRbt2BPdiRpdR6RM6pT4@shrimp.rmq.cloudamqp.com/nqfzvqjw'
params = pika.URLParameters(cloud_url)
connection = pika.BlockingConnection(params)
channel = connection.channel()

def callback(ch, method, properties, body):
    print(" [x] Received %r" % body.decode('UTF-8'), ch, method, properties)

channel.basic_consume(queue='mqtt-subscription-lucasqos0', auto_ack=True, on_message_callback=callback)
print(' [*] Waiting for messages:')
channel.start_consuming()
connection.close()