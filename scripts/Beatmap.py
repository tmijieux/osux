#!/usr/bin/env python2
import sys, re, pprint

def TryFloat(x):
    try:
        return float(x)
    except:
        return x

def TryInt(x):
    try:
        return int(x)
    except:
        return x

def CleanLine(line):
    line = line.rstrip().lstrip()
    return line

class Event:
    def __init__(self, line):
        pass

    def __repr__(self):
        return "Event"
    

class HitObject:
    def ParseSlider(self, values):
        pass
    
    def __init__(self, line):
        values = line.split(",")
        if len(values) < 5:
            raise ValueError("invalid HitObject")
        self.x = int(values[0])
        self.y = int(values[1])
        self.timeOffset = int(values[2])
        self.type = int(values[3])
        self.hitSound = int(values[4])

        if self.type & 4:
            self.newCombo = True
        else:
            self.newCombo = False

        if self.type & 1:
            self.type = "circle"
        elif self.type & 2:
            self.type = "slider"
            self.ParseSlider(values)
        elif self.type & 8:
            self.type = "spinner"
            self.endOffset = int(values[5])
            
    def __repr__(self):
        base = "%d,%d,%d,%s,%d" % (self.x, self.y, self.timeOffset,
                                   self.type, self.hitSound)
        if self.type == "slider":
            pass
        elif self.type == "spinner":
            pass
    
        return base
    
class TimingPoint:
    def __init__(self, line):
        pass
    
    def __str__(self):
        return "TimingPoint"

    
class Beatmap:
    def ParseOsuVersion(self, line):
        matchVersion = re.match(
            u"^(\u00EF\u00BB\u00BF)?osu file format v([0-9]+)$", line)
        if matchVersion:
            if matchVersion.group(1):
                self._BOM = True
            else:
                self._BOM = False
            self._version = int(matchVersion.group(2))
        else:
            #print line
            raise ValueError("Cannot determine '%s' version!" % self._fileName);


    def KeyValueFormat(self, key, value):
        if key == "Bookmarks":
            return map(TryInt, value.split(","))
        if key == "Tags":
            return map(str, value.split(" "))
        return TryFloat(value)
        
    
    def __init__(self, filename):
        self._fileName = filename
        
        if filename.split(".")[-1] != "osu":
            raise ValueError("Invalid file extension")
        f = open(filename)

        line = CleanLine(f.readline())
        self.ParseOsuVersion(line)

        currentSection = None
        self._sections = {}

        lineCounter = 0
        for line in f:
            lineCounter = lineCounter + 1
            line = CleanLine(line)
            
            matchSection = re.match(r"^\[(.*)\]$", line)
            if matchSection:
                currentSection = matchSection.group(1)
                if currentSection in ["TimingPoints", "HitObjects", "Events"]:
                    self._sections[currentSection] = []
                else:
                    self._sections[currentSection] = {}
                continue

            if line == "" or (line[0] =='/' and line[1] == '/'):
                continue # ignore comments or empty lines

            if not(currentSection is None):
                if currentSection == "HitObjects":
                    self._sections[currentSection].append(HitObject(line))
                    continue
                if currentSection == "TimingPoints":
                    self._sections[currentSection].append(TimingPoint(line))
                    continue
                if currentSection == "Events":
                    self._sections[currentSection].append(Event(line))
                    continue
                
                keyValue = map(str.strip, line.split(":", 1))
                if len(keyValue) != 2:
                    raise ValueError("Beatmap Invalid Data line %d" % lineCounter)
                
                key, value = keyValue[0], keyValue[1]
                value = self.KeyValueFormat(key, value)
                self._sections[currentSection][key] = value


    def Print(self):
        pp = pprint.PrettyPrinter(indent=0)
        pp.pprint(self._sections)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "usage %s /path/to/file.osu" % sys.argv[0]
        sys.exit(1)

    try:
        beatmap = Beatmap(sys.argv[1])
        beatmap.Print()
    except ValueError as err:
        print err
    except IOError as err:
        print err
