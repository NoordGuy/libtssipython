libtssipython
=====
libtssipython is a Python interface for libtssi ([https://github.com/goforcode-com/libtssi](https://github.com/goforcode-com/libtssi)), a DVB and MPEG 2 TS demultiplexer and parser.

### Dependencies
- zlib ([http://www.zlib.net](http://www.zlib.net))
- Python (2)
- Boost Python
- libtssi

Usage of CMake ([http://cmake.org](http://cmake.org)) is advisable. A CMake GUI tool may be used to set the dependency paths appropriately. See the CMake help for further information.

### Examples
Let's start by creating a parser object.
```python
>>> import libtssipython
>>> parser = libtssipython.Parser()
```
Suppose a captured TS stream named `stream.ts` with sufficient size is located in the current working directory. We can load and process it.
```python
>>> buffer = bytearray(20*1024*1024)
>>> with open("stream.ts") as file:
...    file.readinto(buffer)
>>> parser.Process(buffer)
```
##### Program Association Table (PAT)
Now we should be able to retrieve some information about the PID mappings of the stream.
```python
>>> pat = parser.TablePat()
>>> pat.GetTransportStreamId()
1073
>>> pat.GetNetworkPid()
16
>>> pat.GetProgramListLength()
27
>>> for x in range(0, 27):
...     ( pat.GetProgramNumber(x), pat.GetProgramMapPid(x) )
(28201, 100)
(28202, 200)
(28203, 300)
(28204, 400)
(28205, 500)
(28206, 600)
(28207, 700)
(28208, 800)
(28210, 1000)
(28211, 1100)
(28212, 1200)
(28214, 1400)
(28215, 1500)
(28216, 1600)
(28218, 1800)
(28219, 1900)
(28221, 2100)
(28222, 2200)
(28224, 2400)
(28225, 2500)
(28226, 2600)
(28227, 2700)
(28228, 2800)
(28229, 2900)
(28230, 3000)
(28231, 3100)
(28250, 5000)
```
Luckily, we do not need to watch for PMT packets manually, libtssi parses PMT information automatically.

##### Program Map Table (PMT)
All PMT information are condensed into a single table.
```python
>>> pmt = parser.TablePmt()
>>> pmt.GetProgramListLength()
27
>>> pmt.GetProgramNumber(3)
28204
>>> pmt.GetPcrPid(28204)
401
>>> pmt.GetEsListLength(28204)
6
>>> for x in range (0, 6):
...     ( pmt.GetEsType(28204, x), pmt.GetEsPid(28204, x) )
(2, 401)
(3, 402)
(5, 470)
(6, 404)
(11, 471)
(11, 2072)
```

##### Service Description Table (SDT)
The former tables are part of MPEG 2 TS and allow for a more general structure analysis of the stream. Additional DVB tables introduce a more TV friendly context.
```python
>>> sdt = parser.TableSdt()
>>> sdt.GetServiceListLength()
26
>>> sdt.GetServiceDescription(3).service_id
28204
>>> dlist = sdt.GetServiceDescription(3).descriptor_list
>>> dlist.GetDescriptorListLength()
1
>>> dlist.GetDescriptor(0)
<libtssipython.Descriptor_Service object at 0x00000000031986C8>
>>> dlist.GetDescriptor(0).GetProviderName()
'ARD'
>>> dlist.GetDescriptor(0).GetServiceName()
'MDR FERNSEHEN'
```

##### Event Information Table (EIT)
To compile a program guide, a table about present and following TV shows is made available.
```python
>>> eit = parser.TableEit()
>>> eit.GetEventListLength()
3898
>>> event = eit.GetEvent(2024)
>>> event.service_id
28007
>>> event.start_time
900519165952L
>>> event.duration
17664
>>> event.descriptor_list.GetDescriptorByTag(0x4d).GetEventName()
'China - Reise durchs Reich der Mitte'
>>> event.descriptor_list.GetDescriptorByTag(0x4d).GetEventText()
'Von Shanghai in die Stadt der Zukunft (2/2) - China entdecken - Ein Land im Wandel?Schweiz 2004'
```

##### EBU Teletext
PES streams are used to carry teletext data. Corresponding PIDs may be found utilizing the PMT information. We have identified PID 404 (stream type 6) in our example. To tell libtssi to watch for teletext on a known PID, inform the parser, and process data afterwards.
```python
>>> parser.SetPidEbu(404)
>>> parser.Process(buffer)
1
>>> teletext = parser.PacketEbu()
>>> teletext.GetCurrentTimeHeader()
'00:06:33'
>>> teletext.GetEbuPageListLength()
102
```
Now, teletext pages can be retrieved (first parameter: magazine number - first digit, second parameter: page number - last two digits, third parameter: sub-page number).
```python
>>> page = teletext.GetEbuPage(3,22,0)
>>> for x in range (0, page.GetPageLineListLength()):
...     print page.GetPageLine(x).GetLineData()
@@@jjj^   @G@@@B    MDR FERNSEHEN
 @pp0p5p
@@@555555@00@        Freitag, 28.10.05
 @!!!#!!
@21.00 - 21.45 Uhr

@Die sch|nsten Jahre meines Lebens
@Mit Judith & Mel

 Dieses Musik-Special vereint die besten
 und erfolgreichsten Lieder von Judith
 und Mel: H|ren Sie "Uns're aller-
 Herzschlag", "Die goldenen Zwanziger",
 "Mitten ins Herz" oder "Die Zeit ver-
 geht". Das sympathische Oldenburger
 Ehepaar widmet sie seinen Fans.

 Seit sehr langer Zeit sind Judith und
 Mel mit ihrer Musik erfolgreich und sie
 gewannen gerade in den letzten beiden
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 Jahren Preise bei diversen Musikwett-
 bewerben.
@@@Jetzt im MDR 333  @            @>317
```
Some characters are not interpreted correctly in the shell.

##### Digital Storage Media Command and Control (DSM-CC) 
To parse file and directoy messages, use the DSM-CC table.
```python
>>> parser.SetPidDsmcc(471)
>>> parser.Process(buffer)
1
>>> dsmcc = parser.TableDsmcc()
>>> dsmcc.GetDownloadListLength()
1
>>> dsmcc.IsDownloadComplete(0)
0
>>> buffer = bytearray(100*1024*1024) # we need more data to process
>>> with open("stream.ts") as file:
...     file.readinto(buffer)
150
>>> parser.Process(buffer)
1
>>> dsmcc.IsDownloadComplete(0)
1
>>> dsmcc.ProcessDownload(0)
1
>>> dsmcc.Decode("pid471_data") # output directory
1
```
A parsed directory structure in `pid471_data` is created in the working directory.

### Author
Written by Martin Hoernig. Visit [goforcode.com](http://goforcode.com) for more information.

### License
libtssipython is GPL-licensed. Please refer to the LICENSE file for detailed information. 
