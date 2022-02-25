const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
      <style>
          html {
           font-family: Arial;
           display: inline-block;
           margin: 0px auto;
           text-align: center;
          }
          h2 { font-size: 3.0rem; }
          p { font-size: 3.0rem; }
          .units { font-size: 1.2rem; }
          .dht-labels{
            font-size: 1.5rem;
            vertical-align:middle;
            padding-bottom: 15px;
          }
      </style>
    </head>
    <body>
        <h2>ESP8266 DHT Server</h2>
        <p>
            <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
            <span class="dht-labels">Temperature</span> 
            <span id="temperature">%TEMPERATURE%</span>
            <sup class="units">&deg;C</sup>
        </p>
        <p>
            <i class="fas fa-tint" style="color:#00add6;"></i> 
            <span class="dht-labels">Humidity</span>
            <span id="humidity">%HUMIDITY%</span>
            <sup class="units">%</sup>
        </p>
        <p>
            <i class="fas fa-wifi" style="color:#00add6;"></i> 
            <span class="dht-labels">Broker:</span>
            <span id="mqtt">#</span>
            <sup class="units"></sup>
        </p>
        <p>
            <i class="fas fa-exclamation-triangle" style="color:#ff0000;"></i> 
            <span class="dht-labels">Qtd Boot:</span>
            <span id="qtd_boot">#</span>
            <sup class="units"></sup>
        </p>          
        <p>
            <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
            <span class="dht-labels">Temp Config:</span>
            <span id="temp_config">#</span>
            <sup class="units">&deg;C</sup>
        </p>  
        <p>
            <form action="/get">
                Nova Temperatura: <input type="text" name="input1" style="font-size:24px" >
                <input type="submit" value="Submit" >
            </form>
        </p>
    </body>
    <script>
            
        setInterval(function ( ) {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              //console.log(this.responseText);
              document.getElementById("mqtt").innerHTML = this.responseText;

              var myArr = JSON.parse(this.responseText);
              document.getElementById("temp_config").innerHTML = myArr.temp_config;
              document.getElementById("qtd_boot").innerHTML = myArr.qtd_boot;
              document.getElementById("humidity").innerHTML = myArr.hum;
              document.getElementById("temperature").innerHTML = myArr.temp;
            }
          };
          xhttp.open("GET", "/mqtt", true);
          xhttp.send();
        }, 5000 ) ;

    </script>
</html>)rawliteral";
