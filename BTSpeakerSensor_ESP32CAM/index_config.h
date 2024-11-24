const char CONFIG_page[]PROGMEM  = R"=====(
<!DOCTYPE html>
<html>

<body>
    <h3> Actuator 1 (Audio detection) </h3><br>
    <button type="button" class="block" onclick="setActuator1State(1)">Turn ON</button>
    <button type="button" class="block" onclick="setActuator1State(0)">Turn OFF</button><br>
    
    <h3> Actuator 2 (Small relay) </h3><br>
    <button type="button" class="block" onclick="setActuator2State(1)">Turn ON Handling</button>
    <button type="button" class="block" onclick="setActuator2State(0)">Turn OFF Handling</button><br>

    <h3> Actuator 3 (Big relay) </h3><br>
    <button type="button" class="block" onclick="setActuator3State(1)">Turn ON Handling</button>
    <button type="button" class="block" onclick="setActuator3State(0)">Turn OFF Handling</button><br>

    Delay start [min] <input type="number" min="1" max="1000" step=1 value="1" oninput="setDelayInterval(this.value)"><br>

<div>
   <br>
   Actuator 1 found: <span id="actuator1Found">NA</span><br>
   Actuator 1 handling: <span id="actuator1State">NA</span><br>
   Actuator 2 found: <span id="actuator2Found">NA</span><br>
   Actuator 2 handling: <span id="actuator2State">NA</span><br>
   Actuator 3 found: <span id="actuator3Found">NA</span><br>
   Actuator 3 handling: <span id="actuator3State">NA</span><br>
   Delayed start [min]: <span id="delayInterval">NA</span><br>
   <br>
</div>


  <!-- <div onload="window.print()"> or window.onload = function() { window.print(); } -->

  <script>
    
    window.onload = function() {
      getState();
    };

    function getState(){
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var res = this.responseText.split("<br>");
          // There should be <br> at the end
          for (i=0; i < res.length-1; i++)
          {
            var resInner = res[i].split("=");
            document.getElementById(resInner[0]).innerHTML = resInner[1];
          }
        }
      };
      xhttp.open("GET", "getState", false);
      xhttp.send();
    }

    function setActuator1State(value) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("actuator1State").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setActuator1State?state=" + value, true);
      xhttp.send();
    }

    function setActuator2State(value) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("actuator2State").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setActuator2State?state=" + value, true);
      xhttp.send();
    }

    function setActuator3State(value) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("actuator3State").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setActuator3State?state=" + value, true);
      xhttp.send();
    }

    function setDelayInterval(value) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("delayInterval").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setDelayInterval?delay=" + value, true);
      xhttp.send();
    }

   


  </script>
</body>

<style>
.block {
  display: block;
  width: 20%;
  border: none;
  background-color: #04AA6D;
  color: white;
  padding: 14px 28px;
  font-size: 16px;
  cursor: pointer;
  text-align: center;
}

.block:hover {
  background-color: #ddd;
  color: black;
}

 /* The switch - the box around the slider */
.switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}

/* Hide default HTML checkbox */
.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

/* The slider */
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
} 
</style>

</html>
)=====";
