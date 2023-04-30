// File definitions
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    debugln("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    debugln(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      debug("  DIR : ");
      debugln(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      debug("  FILE: ");
      debug(file.name());
      debug("\tSIZE: ");
      debugln(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    debugln("- failed to open file for reading");
    return;
  }

  debugln("- read from file:");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    debugln("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    debugln("- file written");
  }
  else
  {
    debugln("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  //   Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    debugln("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    //        Serial.print("- message appended");
  }
  else
  {
    debugln("- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    debugln("- file renamed");
  }
  else
  {
    debugln("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    debugln("- file deleted");
  }
  else
  {
    debugln("- delete failed");
  }
}
