
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
    
    float Vin = analogRead(BATT_VOLTS);
    float Vout = (R2/(R1+R2))*Vin;
    Vout = Vin * .0107;

//    Serial.println("Battery Voltage");
//    Serial.print("Vin :");
//    Serial.print(Vin);
//    Serial.print("\tVout");
//    Serial.println(Vout);
    
    return Vout;
}


