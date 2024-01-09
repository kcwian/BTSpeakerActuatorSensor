const char MAIN_page[] = R"=====(
<!DOCTYPE html>
<html>

<body>
    <button type="button" class="block" onclick="setState(1)">Turn ON</button><br>
    <button type="button" class="block" onclick="setState(0)">Turn OFF</button><br>
<div>
   <br>
   State is : <span id="state">NA</span><br>
   Audio Level is : <span id="audioLevel">NA</span><br>
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

  </script>
</body>

<style>
.block {
  display: block;
  width: 50%;
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
</style>

</html>
)=====";
