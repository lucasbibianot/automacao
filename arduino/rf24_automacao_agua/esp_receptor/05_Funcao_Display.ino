 void display_listenning(String &msg) {
  if (byte(config["usar_display"] == "1")) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("msg: ");
    lcd.setCursor(5,0);
    lcd.print(msg);
  }
}

 void display_ip(String &strIP) {
  if (byte(config["usar_display"] == "1")) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("IP:");
    lcd.setCursor(0,1);
    lcd.print(strIP);
  }
}


void display_error(String &strMsgErro)  {
  if (byte(config["usar_display"] == "1")) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Error: ");
    lcd.setCursor(0,1);
    lcd.print(strMsgErro);
  }
}

void display_vazio() {
  if (byte(config["usar_display"] == "1")) {
    lcd.clear();
  }
}

void display_msg(String strMsg1, String strMsg2) {
  if (byte(config["usar_display"] == "1")) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(strMsg1);
    lcd.setCursor(0,1);
    lcd.print(strMsg2);
  }
}
