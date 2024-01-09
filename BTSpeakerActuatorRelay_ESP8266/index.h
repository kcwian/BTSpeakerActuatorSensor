const char MAIN_page[]PROGMEM  = R"=====(
<!DOCTYPE html>
<html>

<body>
    <button type="button" class="block" onclick="setState(1)">Turn ON</button><br>
    <button type="button" class="block" onclick="setState(0)">Turn OFF</button><br>
    <label class="switch"> <input type="checkbox" onchange=checkSensorHandler(this)> <span class="slider round"> </span> </label> Check sensor periodically <br>
<div>
   <br>
   User State is : <span id="state">NA</span><br>
   Audio State is : <span id="audioState">NA</span><br>
   Check sensor is: <span id="checkSensor">NA</span><br>
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

    function setState(value) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("state").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setState?state=" + value, true);
      xhttp.send();
    }

    function checkSensorHandler(box)
    {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("checkSensor").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "setCheckSensor?checkSensor=" + (box.checked * 1), true);
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
