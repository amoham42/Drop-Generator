#ifndef INDEX__HTML_H
#define INDEX__HTML_H

const char main_page[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
  <head>
    <title>Drop-Gen Controller</title>
    <link rel="icon" href="hlogo.png" type="image/x-icon">
    <link rel="stylesheet" type="text/css" href="config.css">
  </head>
  <body>
    <span3>
      <span>
        <div class="dr-container">
          <label>Reservoir</label>
          <label>Main</label>
          <label>Drop Gen</label></div>
        <div class="dr-container">
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(1)'>&#11014;</button>
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(6)'>&#11014;</button>
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(2)'>&#11014;</button></div>
        <div class="dr-container">
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(3)'>&#11015;</button>
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(5)'>&#11015;</button>
          <button style='background-color: #187bcd; margin: 5px;' type='button' onclick='mtr(4)'>&#11015;</button></div>
        <button style='margin-bottom: 10px;' type='button' onclick='mtr(0)'>Calibrate</button>
        <div class="dr-container">
          <label>Drop Gen:</label>
          <div class="wrapper"><div class="water"><div class="drop"></div></div></div>
          <button id='toggle-gen' onclick='mainToggle("/dropgen")'>Off</button></div>
        <div class="dr-container">
          <label>Camera:</label>
          <div class="container">
            <input type="checkbox" id="camera" disabled>
            <label for="camera" class="cam"><img src="camera.png"><div class="line"></div></label></div>
          <button id='toggle-cam' onclick='mainToggle("/camera")'>Off</button></div>
      </span>
      <span style='padding: 18px; margin-top: 5px'>
        <div style='height: 200px; flex-direction: column;' class="dr-container">
          <div class="label-container">
            <div style="margin-right: 10px;" class="rectangle"></div>
            <label style="margin-right: 10px;">Pulse Width</label></div>
          <canvas id="graph"></canvas></div></span>
    </span3>
    <span><span2><div class="dr-container">
        <label for='textbox1'>Pulse Width (µs):</label>
        <input type='number' id='textbox1'></div>
        <div><span1 id='plsw'>-</span1></div></span2>
      <span2><div class="dr-container">
        <label for='textbox2'>Camera Delay (ms):</label>
        <input type='number' id='textbox2'></div>
        <div><span1 id='cmdel'>-</span1></div></span2>
      <span2><div class="dr-container">
        <label for='textbox3'>Droplet Delay (ms):</label>
        <input type='number' id='textbox3'></div>
        <div><span1 id='dpdel'>-</span1></div></span2>
      <span2><div class="dr-container">
        <label for='textbox4'>Droplet Number:</label>
        <input type='number' id='textbox4'></div>
        <div><span1 id='dpnum'>-</span1></div></span2>
      <span2><div class="dr-container">
        <label for='textbox5'>Travel Length (mm):</label>
        <input type='number' id='textbox5'></div>
        <div><span1 id='trvl'>-</span1></div></span2>
      <button type='button' id='SEND_DATA' onclick='send_data()'>Submit</button>
    </span>
    <script src="database.js"></script>
  </body>
</html>

)=====";
#endif
