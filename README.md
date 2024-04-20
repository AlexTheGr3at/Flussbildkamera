# Flussbildkamera
Dieser Code erfasst Bilder in verschiedenen Abständen und übermittelt sie mithilfe einer SIM800L-Modulverbindung an eine spezifische URL über einen Http-Post-Request. Um ein Foto zu machen und es auf einen vorgegebenen Webserver anzuzeigen – insgesamt zwölf Mal pro Tag – befindet sich der ESP größtenteils im Deep-Sleep-Modus. Der ESP wird im Deep-Sleep-Modus zwischen 18:00 und 6:00 Uhr morgens ständig betrieben, um Strom zu sparen. 
Die SIM800L-Verbindung ist das einzige Problem, da sie nicht funktioniert. 
Aber es gibt genau denselben Code, der stattdessen mit einer einfachen WLAN-Verbindung stattfindet.
