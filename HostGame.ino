//full internal hosting from esp32 module
// returns the taggerID when joined

// need a way to abort it when running.

int hostPlayerToGame(int playerNumber, int _gameType, int _gameID, int _gameLength, int _health, int _reloads, int _shields, int _megaTags, int _flags1, int _flags2, int _interval)
{
    Serial.print("hostPlayerToGame");
    
    uint16_t _taggerID = -1;    // -1 means failed !

    if(cancelHosting) isHostingActive = false;
    
    //  if(interval > millis() ) sendHostGame
    //  if(irReply.startsWith "P16" respond
    //    else if(irReply.startsWith "P17" respond
        




    return _taggerID;
}

