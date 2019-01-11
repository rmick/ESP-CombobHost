bool checkBattery(bool updateDisplay)
{
    static int oldLedIntensity = LED_INTENSITY;
    static bool result = true;
    if(millis() - batteryTestTimer > 5000)
    {
        batteryTestTimer = millis();

        float battVolts = BatteryVoltage();
        
        if(battVolts < LOW_BATT_VOLTS)
        {
            oldLedIntensity = ledIntensity;
            ledIntensity = 1;
            rgbLED(1,0,0);
            if (updateDisplay)
            {
                writeDisplay("Replace",                             2, CENTRE_HOR, 2, true, false);
                writeDisplay("Batteries",                           2, CENTRE_HOR, 3, false, true);
                displayBatteryVoltage(battVolts);
            }
            result = false;
        }
        else
        {
            result = true;
            ledIntensity = oldLedIntensity;
            if (updateDisplay)  displayBatteryVoltage(battVolts);
            rgbLED(0,0,0);
        }
    }
    return result;
}


float BatteryVoltage()
{
    //Formula   Vout = ((R1+R2/R2)*Vin
    //
    //          R1 + R2
    //  Vout =  -------  * Vin
    //             R2
    //
    //  Vout = 0.80645 * Vin
    
    #define     RESISTOR_1          5000.0
    #define     RESISTOR_2          1200.0
    #define     NUM_SAMPLES         5
    int         sampleSum           = 0;
    int         sampleCount         = 0;

    while (sampleCount < NUM_SAMPLES)
    {
        sampleSum += analogRead(BATT_VOLTS);
        sampleCount++;
        delay(2);
    }
    unsigned int adcValue = sampleSum/sampleCount;
    
    float dividedVoltage    = (adcValue * 3.3) / 1023.0;
    float calculatedVoltage = (dividedVoltage / (RESISTOR_1/(RESISTOR_1 + RESISTOR_2)) * 5);
    int   roundedVoltage    = calculatedVoltage * 10;
    float voltage = roundedVoltage / 10.0;

    client.println("BATT,"  + String(voltage) + ",@");
    Serial.println("BATT: " + String(voltage));
    return voltage;
    
}

void displayBatteryVoltage(float volts)
{
    String battVolts = String(volts);
    battVolts.remove(battVolts.length()-1, 1);
    writeDisplay("Battery = " + battVolts + "v", 1, CENTRE_HOR, 8,  false, true);
}
