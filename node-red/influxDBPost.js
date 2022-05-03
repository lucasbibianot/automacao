const { InfluxDB } = InfluxClient;
// You can generate an API token from the "API Tokens Tab" in the UI
const token =
  "wsCGjok61f5UJGKmY3YYL2b3rNnyg69deuhQ5hDXQNIYyi19RM6RCP4gctAV8b51Kk6ptwgy9zoGeK3R6K7N1w==";
const org = "lucasvbt@trt3.jus.br";
const bucket = "lucasvbt's Bucket";

const client = new InfluxDB({
  url: "https://us-east-1-1.aws.cloud2.influxdata.com",
  token: token,
});
const { Point } = InfluxClient;
const writeApi = client.getWriteApi(org, bucket);
let metaInfo = {topic: msg.topic};

Object.entries(msg.payload).map((item) => {
  if ("C" === item[1].tipo) {
    metaInfo[item[0]] = item[1].valor;
  }
});

writeApi.useDefaultTags(metaInfo);
//node.warn(metaInfo);

Object.entries(msg.payload).map((item) => {
  if ("M" === item[1].tipo) {
    const point = new Point(item[0]).floatField(item[0], item[1].valor);
    writeApi.writePoint(point);
  }
});


writeApi
  .close()
  .then(() => {
    node.log("FINISHED");
  })
  .catch((e) => {
    node.error(e);
    node.log("Finished ERROR");
  });

return msg;
