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
            <h2>ESP8266 DHT Server</h2>
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
