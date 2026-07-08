int main()
{
    display.begin();

    wifi.begin();

    mqtt.begin();

    gui.begin();

    while (true)
    {
        wifi.loop();
        mqtt.loop();
        gui.loop();
    }
}