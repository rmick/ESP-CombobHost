
float BatteryVoltage()
{
    #define R1  10000
    #define R2  1200

    //Formula   Vout = (R2/(R1+R2))*Vin
    //
    //            R2
    //  Vout =  -------  * Vin
    //          R1 + R2
    //
    //  Vout = 0.107142 * Vin
    
    unsigned int adcValue = analogRead(BATT_VOLTS);
    //Serial.print("ADC reading is ");
    //Serial.println(adcValue);
    float dividedVoltage = (adcValue * 3.3) / 1023.0;
    //Serial.print("Input  voltage is ");
    //Serial.println(dividedVoltage);
    //Serial.print("Battery voltage should be ");
    float calculatedVoltage = dividedVoltage / (5000.0/(1200.0+5000.0));
    //Serial.println(calculatedVoltage * 5.000);

    return ((calculatedVoltage * 5.000) *10) /10.0;

    
    float Vout = (R2/(R1+R2))*adcValue;


}

bool checkBattery()
{
    static bool result = true;
    if(millis() - batteryTestTimer > 2000)
    {
        batteryTestTimer = millis();
        
        if(BatteryVoltage() < 3.0)
        {
            rgbLED(1,0,0);
            writeDisplay("Replace",   2, CENTRE_HOR, 2, true,  true);
            writeDisplay("Batteries", 2, CENTRE_HOR, 3, false, true);
            writeDisplay("Volts = " + String(BatteryVoltage()), 1, CENTRE_HOR, 8, false, true);
            Serial.println("***** FLAT BATTERY *****");
            result = false;
        }
        else
        {
            result = true;
            rgbLED(0,0,0);
        }
    }
    return result;
}
