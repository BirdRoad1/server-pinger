#include "scanner.cpp"
#include "database.cpp"
#include <optional>

int main()
{
    Database db;

    if (!db.connect())
    {
        return 1;
    }

    try
    {
        db.setupDatabase();
    }
    catch (std::exception &err)
    {
        std::cout << "Failed to setup database: " << err.what() << std::endl;
    }

    startScanner(&db);
    return 0;
}