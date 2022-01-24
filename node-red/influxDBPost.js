
const {InfluxDB} = InfluxClient
// You can generate an API token from the "API Tokens Tab" in the UI
const token = 'wsCGjok61f5UJGKmY3YYL2b3rNnyg69deuhQ5hDXQNIYyi19RM6RCP4gctAV8b51Kk6ptwgy9zoGeK3R6K7N1w==';
const org = 'lucasvbt@trt3.jus.br';
const bucket = 'lucasvbt\'s Bucket';

const client = new InfluxDB({url: 'https://us-east-1-1.aws.cloud2.influxdata.com', token: token});
const {Point} = InfluxClient
const writeApi = client.getWriteApi(org, bucket)
writeApi.useDefaultTags({host: 'host1', topic: msg.topic})

const point = new Point('sensor').floatField('temp', msg.payload.temp)
.floatField('hum', msg.payload.hum)
const pointTemp = new Point('temperatura').floatField('temp', msg.payload.temp)
const pointHum = new Point('humidade').floatField('hum', msg.payload.hum)

writeApi.writePoint(point)
writeApi.writePoint(pointTemp)
writeApi.writePoint(pointHum)


writeApi
    .close()
    .then(() => {
        node.log('FINISHED')
    })
    .catch(e => {
        node.error(e)
        node.log('Finished ERROR')
    })

return msg;
