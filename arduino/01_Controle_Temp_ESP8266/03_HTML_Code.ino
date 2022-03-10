const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            border: 0;
            font-family: Arial;
        }

        body {
            background: #f1f1f1;
        }

        header {
            display: flex;
            justify-content: center;
            margin: 2rem 0 4rem 0;
        }

        main {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        section {
            display: flex;
            flex-direction: column;
            justify-content: left;
        }

        span {

            font-size: 2rem;
        }

        div .items {
            display: flex;
            margin-bottom: 2rem;
        }

        div .icon {
            display: flex;
            justify-content: center;
            align-items: center;
            margin-right: 1rem;
            width: 4.37rem;
            font-size: 3.0rem;
        }

        h2 {
            font-size: 3.0rem;
        }

        div .content {
            display: flex;
            align-items: center;
            gap: 1rem;
            font-size: 3.0rem;
        }

        .units {
            font-size: 1.5rem;
        }

        .dht-labels {
            font-size: 1.5rem;
        }

        form {
            display: flex;
            flex-direction: column;
            gap: 1rem;
            margin-top: 2rem;
        }

        form label {
            font-size: 1.2rem;
        }

        form input {
            border-radius: .5rem;
            padding: 1rem 2rem;
            font-size: 1rem;
        }

        form button {
            font-size: 1rem;
            padding: .8rem;
            border-radius: .5rem;
            background: #5b88a5;
            color: #fff;
            cursor: pointer;
        }

        form button:hover {
            filter: brightness(.9);
        }
    </style>
</head>

<body>
    <div class="container">
        <header>
            <h2>Controle Bercario Granja</h2>
        </header>

        <main>
            <section>
                <div class="items">
                    <div class="icon">
                        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
                    </div>
                    <div class="content">
                        <span class="dht-labels">Temperature</span>
                        <span id="temperature">%TEMPERATURE%</span>
                        <span class="units">&deg;C</span>
                    </div>
                </div>
                <div class="items">
                    <div class="icon">
                        <i class="fas fa-tint" style="color:#00add6;"></i>
                    </div>
                    <div class="content">
                        <span class="dht-labels">Humidity</span>
                        <span id="humidity">%HUMIDITY%</span>
                        <span class="units">%</span>
                    </div>
                </div>
                <!--
                <div class="items">
                    <div class="icon">
                        <i class="fas fa-wifi" style="color:#00add6;"></i>
                    </div>
                    <div class="content">
                        <span class="dht-labels">Broker:</span>
                        <span id="mqtt">#</span>
                        <span class="units"></span>
                    </div>
                </div>
                -->
                <div class="items">
                    <div class="icon">
                        <i class="fas fa-exclamation-triangle" style="color:#ff0000;"></i>
                    </div>
                    <div class="content">
                        <span class="dht-labels">Qtd Boot:</span>
                        <span id="qtd_boot">#</span>
                        <span class="units"></span>
                    </div>
                </div>
                <div class="items">
                    <div class="icon">
                        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
                    </div>
                    <div class="content">
                        <span class="dht-labels">Temp Config:</span>
                        <span id="temp_config">#</span>
                        <span class="units">&deg;C</span>
                    </div>
                </div>
            </section>
            <div class="form">
                <form action="/get">
                    <label>Nova Temperatura: </label>
                    <input type="number" name="input1" placeholder="Digite a nova temperatura">
                    <button type="submit" class="submit">Enviar</button>
                </form>
            </div>
        </main>
    </div>
</body>
<script>
    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                //console.log(this.responseText);
                //document.getElementById("mqtt").innerHTML = this.responseText;

                var myArr = JSON.parse(this.responseText);
                document.getElementById("temp_config").innerHTML = myArr.temp_config;
                document.getElementById("qtd_boot").innerHTML = myArr.qtd_boot;
                document.getElementById("humidity").innerHTML = myArr.hum;
                document.getElementById("temperature").innerHTML = myArr.temp;
            }
        };
        xhttp.open("GET", "/mqtt", true);
        xhttp.send();
    }, 5000);
</script>
</html>)rawliteral";

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            border: 0;
            font-family: Arial;
        }

        body {
            background: #f1f1f1;
        }

        header {
            display: flex;
            justify-content: center;
            margin-top: 2rem;
        }

        main {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        section {
            display: flex;
            flex-direction: column;
            justify-content: left;
        }

        span {
            font-size: 2rem;
        }

        div .items {
            display: flex;
            align-items: center;
            margin-bottom: 0rem;
        }

        div .items {
            display: flex;
            justify-content: center;
            gap: 1rem;
        }

        div .items .fas.fa-wifi {
            font-size: 2rem;
        }

        h2 {
            font-size: 3.0rem;
        }

        div .titulo {
            display: flex;
            align-items: left;
            gap: 0.5rem;
            font-size: 1.0rem;
            width: 10.0rem;
        }

        div .content {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            font-size: 3.0rem;
            width: 40.0rem;
        }

        div .content_button {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 1rem;
            font-size: 3.0rem;
            width: 40.0rem;
        }

        .units {
            font-size: 1.5rem;
        }

        .dht-labels {
            font-size: 1.5rem;
        }

        form {
            display: flex;
            flex-direction: column;
            gap: 1rem;
            margin-top: 2rem;
        }

        form label {
            font-size: 1.2rem;
        }

        form input {
            border-radius: .5rem;
            padding: 1rem 2rem;
            font-size: 1rem;
        }

        form .items:last-child {
            margin: 2rem 0;
        }

        form .input_string {
            border-radius: .5rem;
            padding: 1rem 2rem;
            font-size: 1rem;
            width: 35.0rem;
        }

        form .input_number {
            border-radius: .5rem;
            padding: 1rem 2rem;
            font-size: 1rem;
            width: 9.0rem;
        }

        form button {
            font-size: 1rem;
            padding: .8rem;
            border-radius: .5rem;
            background: #5b88a5;
            color: #fff;
            cursor: pointer;
            width: 20.0rem;
        }

        form button:hover {
            filter: brightness(.9);
        }
    </style>
</head>
<body onload="(read_config())">    
    <div class="container">
        <header>
            <div class="items" style="justify-content: center;">
                <i class="fas fa-wifi" style="color:#00add6;"></i>
                <h1>Configuracao</h1>
            </div>
        </header>
        <main>
            <section>            
                <div class="form">
                    <form action="/config" method="POST">
                        <div class="items">
                            <div class="titulo">
                                <label>Wifi-ssid: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_wifi_ssid" name="input_wifi_ssid" placeholder="Digite o ssid">
                            </div>
                        </div>   
                        <div class="items">
                            <div class="titulo">
                                <label>Wifi-key: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_wifi_key" name="input_wifi_key" placeholder="Digite a chave">
                            </div>
                        </div>   
                        <div class="items">
                            <div class="titulo">
                                <label>Topic: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_topic" name="input_topic" placeholder="Digite o topic">
                            </div>
                        </div>   
                        <div class="items">
                            <div class="titulo">
                                <label>Topic subscribe: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_topic_subscribe" name="input_topic_subscribe" placeholder="Digite o topic">
                            </div>
                        </div>   
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt server: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_server" name="input_mqtt_server" placeholder="Digite a string de conexao">
                            </div>
                        </div>                    <div class="items">
                            <div class="titulo">
                                <label>Mqtt server port: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_number" type="number" id="input_mqtt_server_port" name="input_mqtt_server_port" placeholder="Porta">
                            </div>
                        </div>                                                                                                 
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt Tag1: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_tag1" name="input_mqtt_tag1" placeholder="Digite a nova Tag">
                            </div>
                        </div>            
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt Tag2: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_tag2" name="input_mqtt_tag2" placeholder="Digite a nova Tag">
                            </div>
                        </div>                       
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt Client Id: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_client_id" name="input_mqtt_client_id" placeholder="Digite Mqtt ID">
                            </div>
                        </div>                    
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt user: &nbsp;</label>
                            </div>
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_user" name="input_mqtt_user" placeholder="Digite o usuario">
                            </div>
                        </div>
                        <div class="items">
                            <div class="titulo">
                                <label>Mqtt password: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_mqtt_password" name="input_mqtt_password" placeholder="Digite o password">
                            </div>
                        </div>   
                        <div class="items">
                            <div class="titulo">
                                <label>Soft-ap: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_string" type="string" id="input_soft_ap" name="input_soft_ap" placeholder="Digite o nome da rede">
                            </div>
                        </div>                                                                                                 
                        <div class="items">
                            <div class="titulo">
                                <label>Temperatura: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_number" type="number" id="input_temp" name="input_temp" placeholder="Temp">
                            </div>
                        </div>            
                        <div class="items">
                            <div class="titulo">
                                <label>Qtd boot: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_number" type="number" id="input_qtd_boot" name="input_qtd_boot" placeholder="" readonly>
                            </div>
                        </div>                       
                        <div class="items">
                            <div class="titulo">
                                <label>Interval: &nbsp;</label>
                            </div>                        
                            <div class="content">
                                <input class="input_number" type="number" id="input_interval" name="input_interval" placeholder="Intervalo">
                            </div>
                        </div>                    
                        <div class="items">
                            <div class="titulo">
                                <label>Interval mqtt: &nbsp;</label>
                            </div>
                            <div class="content">
                                <input class="input_number" type="number" id="input_interval_mqtt" name="input_interval_mqtt" placeholder="Intervalo">
                            </div>
                        </div>             
                        <div class="items">
                            <div class="titulo">
                                <label>Usar display: &nbsp;</label>
                            </div>
                            <div class="content">
                                <input class="input_number" type="number" id="input_usar_display" name="input_usar_display" placeholder="1 ou 0">
                            </div>
                        </div>                               
                        <div class="items">
                            <div class="content_button">
                                <button type="submit" class="submit">Enviar</button>
                            </div>
                        </div>                    
                    </form>
                </div>
            </section>
        </main>
    </div>
</body>
<script>
    function read_config(){
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                //console.log(this.responseText);
                //document.getElementById("mqtt").innerHTML = this.responseText;

                var myArr = JSON.parse(this.responseText);
                document.getElementById("input_wifi_ssid").value = myArr.wifi_ssid;
                document.getElementById("input_wifi_key").value = myArr.wifi_key;        
                document.getElementById("input_topic").value = myArr.topic;
                document.getElementById("input_topic_subscribe").value = myArr.topic_subscribe;
                document.getElementById("input_mqtt_server").value = myArr.mqtt_server;
                document.getElementById("input_mqtt_server_port").value = myArr.mqtt_server_port;
                document.getElementById("input_mqtt_tag1").value = myArr.mqtt_tag1;
                document.getElementById("input_mqtt_tag2").value = myArr.mqtt_tag2;
                document.getElementById("input_mqtt_client_id").value = myArr.mqtt_client_id;
                document.getElementById("input_mqtt_user").value = myArr.mqtt_user;
                document.getElementById("input_mqtt_password").value = myArr.mqtt_password;
                document.getElementById("input_soft_ap").value = myArr.soft_ap;
                document.getElementById("input_temp").value = myArr.temp;
                document.getElementById("input_qtd_boot").value = myArr.qtd_boot;
                document.getElementById("input_interval").value = myArr.interval;
                document.getElementById("input_interval_mqtt").value = myArr.interval_mqtt;
                document.getElementById("input_usar_display").value = myArr.usar_display;
            }
        };
        xhttp.open("GET", "/read_config", true);
        xhttp.send();
    }
</script>
</html>
)rawliteral";
