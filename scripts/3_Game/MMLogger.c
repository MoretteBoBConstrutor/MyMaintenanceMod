class MMLogger
{
    private static const string LOG_DIR = "$profile:MaintenanceMod";
    private static const string LOG_FILE = LOG_DIR + "/activity.log";

    // Escreve uma mensagem no arquivo de log customizado do mod
    static void Log(string message)
    {
        // O logging em arquivo deve ocorrer apenas no lado do servidor
        if (!GetGame().IsServer())
            return;

        // Garante que o diretório do mod no perfil do servidor exista
        if (!FileExist(LOG_DIR))
        {
            MakeDirectory(LOG_DIR);
        }

        FileHandle fileHandle = OpenFile(LOG_FILE, FileMode.APPEND);
        if (fileHandle == 0)
            return; // Não foi possível abrir o arquivo

        int year, month, day, hour, minute, second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);

        // Formata o timestamp para um padrão legível
        string timestamp = "[" + year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + " " + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "]";
        
        FPrintln(fileHandle, timestamp + " " + message);

        CloseFile(fileHandle);
    }
}