 void display_temp_hum(float &fltTemp, float &fltHum) {
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1  
    lcd.print("Temp: "); 
    lcd.setCursor(0,1);
    lcd.print("Humidade: "); 
    //lcd.setCursor(6,0); 
    //lcd.print("      ");   
    lcd.setCursor(6,0); 
    lcd.print(fltTemp);   
    //lcd.setCursor(10,1); 
    //lcd.print("      ");   
    lcd.setCursor(10,1); 
    lcd.print(fltHum);     
 }
 
 void display_ip(String &strIP) {
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1  
    lcd.print("IP: "); 
    lcd.setCursor(4,0); 
    lcd.print(strIP);   
 }

void display_temp_config(float fltTempConfig, String strQtdBoot){
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1  
    lcd.print("Temp Conf: "); 
    lcd.setCursor(11,0); 
    lcd.print(String(fltTempConfig).c_str());     
    lcd.setCursor(0,1); 
    lcd.print("Qtd Boot: "); 
    lcd.setCursor(10,1); 
    lcd.print(strQtdBoot);
} 

void display_error(String &strMsgErro)  {
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1  
    lcd.print("Error: "); 
    lcd.setCursor(0,1); 
    lcd.print(strMsgErro);     
}

 void display_vazio() {
    lcd.clear(); //limpa o display    
 } 

void display_msg(String strMsg1, String strMsg2) {
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1
    lcd.print(strMsg1);
    lcd.setCursor(0,1); //posiciona o cursor na primeira coluna da linha 2
    lcd.print(strMsg2);          
}
