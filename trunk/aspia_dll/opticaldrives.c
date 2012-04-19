/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/opticaldrives.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


typedef struct
{
    BOOL CD_ROM_Read;
    BOOL CD_ROM_Write;

    BOOL CD_R_Read;
    BOOL CD_R_Write;

    BOOL CD_RW_Read;
    BOOL CD_RW_Write;

    BOOL DVD_ROM_Read;
    BOOL DVD_ROM_Write;

    BOOL DVD_RAM_Read;
    BOOL DVD_RAM_Write;

    BOOL DVD_P_R_Read;
    BOOL DVD_P_R_Write;

    BOOL DVD_P_RW_Read;
    BOOL DVD_P_RW_Write;

    BOOL DVD_R_Read;
    BOOL DVD_R_Write;

    BOOL DVD_RW_Read;
    BOOL DVD_RW_Write;

    BOOL DVD_R_DL_Read;
    BOOL DVD_R_DL_Write;

    BOOL DVD_RW_DL_Read;
    BOOL DVD_RW_DL_Write;

    BOOL DVD_P_R_DL_Read;
    BOOL DVD_P_R_DL_Write;

    BOOL DVD_P_RW_DL_Read;
    BOOL DVD_P_RW_DL_Write;

    BOOL HD_DVD_ROM_Read;
    BOOL HD_DVD_ROM_Write;

    BOOL HD_DVD_RAM_Read;
    BOOL HD_DVD_RAM_Write;

    BOOL HD_DVD_R_Read;
    BOOL HD_DVD_R_Write;

    BOOL HD_DVD_RW_Read;
    BOOL HD_DVD_RW_Write;

    BOOL HD_DVD_R_DL_Read;
    BOOL HD_DVD_R_DL_Write;

    BOOL HD_DVD_RW_DL_Read;
    BOOL HD_DVD_RW_DL_Write;

    BOOL BD_ROM_Read;
    BOOL BD_ROM_Write;

    BOOL BD_R_Read;
    BOOL BD_R_Write;

    BOOL BD_RE_Read;
    BOOL BD_RE_Write;
} CD_READ_WRITE_SUPPORT, *PCD_READ_WRITE_SUPPORT;

typedef struct
{
    CHAR *pVendorId;
    WCHAR *pVendorName;
} SCSI_VENDORS;

SCSI_VENDORS ScsiVendors[] =
{
    { "0B4C", L"MOOSIK Ltd." },
    { "2AI", L"2AI (Automatisme et Avenir Informatique)" },
    { "3M", L"3M Company" },
    { "3nhtech", L"3NH Technologies" },
    { "3PARdata", L"3PARdata, Inc. (now HP)" },
    { "A-Max", L"A-Max Technology Co., Ltd" },
    { "ABSOLUTE", L"Absolute Analysis" },
    { "ACARD", L"ACARD Technology Corp." },
    { "Accusys", L"Accusys INC." },
    { "Acer", L"Acer, Inc." },
    { "ACL", L"Automated Cartridge Librarys, Inc." },
    { "Acuid", L"Acuid Corporation Ltd." },
    { "AcuLab", L"AcuLab, Inc. (Tulsa, OK)" },
    { "ADAPTEC", L"Adaptec (now PMC-Sierra)" },
    { "ADIC", L"Advanced Digital Information Corporation" },
    { "ADSI", L"Adaptive Data Systems, Inc. (a Western Digital subsidiary)" },
    { "ADTX", L"ADTX Co., Ltd." },
    { "ADVA", L"ADVA Optical Networking AG" },
    { "AEM", L"AEM Performance Electronics" },
    { "AERONICS", L"Aeronics, Inc." },
    { "AGFA", L"AGFA" },
    { "AIC", L"Advanced Industrial Computer, Inc." },
    { "AIPTEK", L"AIPTEK International Inc." },
    { "ALCOR", L"Alcor Micro, Corp." },
    { "AMCC", L"Applied Micro Circuits Corporation" },
    { "AMCODYNE", L"Amcodyne" },
    { "Amgeon", L"Amgeon LLC" },
    { "AMI", L"American Megatrends, Inc." },
    { "Amphenol", L"Amphenol" },
    { "Amtl", L"Tenlon Technology Co.,Ltd" },
    { "ANAMATIC", L"Anamartic Limited (England)" },
    { "Ancor", L"Ancor Communications, Inc." },
    { "ANCOT", L"ANCOT Corp." },
    { "ANDATACO", L"Andataco  (now nStor)" },
    { "andiamo", L"Andiamo Systems, Inc." },
    { "ANOBIT", L"Anobit" },
    { "ANRITSU", L"Anritsu Corporation" },
    { "ANTONIO", L"Antonio Precise Products Manufactory Ltd." },
    { "AoT", L"Art of Technology AG" },
    { "APPLE", L"Apple Computer, Inc." },
    { "ARCHIVE", L"Archive" },
    { "ARDENCE", L"Ardence Inc" },
    { "Areca", L"Areca Technology Corporation" },
    { "Arena", L"MaxTronic International Co., Ltd." },
    { "ARIO", L"Ario Data Networks, Inc." },
    { "ARISTOS", L"Aristos Logic Corp. (now part of Adaptec)" },
    { "ARK", L"ARK Research Corporation" },
    { "ARTECON", L"Artecon Inc.  (Obs. - now Dot Hill)" },
    { "Artistic", L"Artistic Licence (UK) Ltd" },
    { "ARTON", L"Arton Int." },
    { "ASACA", L"ASACA Corp." },
    { "ASC", L"Advanced Storage Concepts, Inc." },
    { "ASPEN", L"Aspen Peripherals" },
    { "AST", L"AST Research" },
    { "ASTEK", L"Astek Corporation" },
    { "ASTK", L"Alcatel STK A/S" },
    { "ASTUTE", L"Astute Networks, Inc." },
    { "AT&T", L"AT&T" },
    { "ATA", L"SCSI / ATA Translator Software (Organization Not Specified)" },
    { "ATARI", L"Atari Corporation" },
    { "ATech", L"ATech electronics" },
    { "ATG CYG", L"ATG Cygnet Inc." },
    { "ATL", L"Quantum|ATL Products" },
    { "ATTO", L"ATTO Technology Inc." },
    { "ATTRATEC", L"Attratech Ltd liab. Co" },
    { "ATX", L"Alphatronix" },
    { "AURASEN", L"Aurasen Limited" },
    { "AVC", L"AVC Technology Ltd" },
    { "AVIDVIDR", L"AVID Technologies, Inc." },
    { "AVR", L"Advanced Vision Research" },
    { "AXSTOR", L"AXSTOR" },
    { "BALLARD", L"Ballard Synergy Corp." },
    { "Barco", L"Barco" },
    { "BAROMTEC", L"Barom Technologies Co., Ltd." },
    { "Bassett", L"Bassett Electronic Systems Ltd" },
    { "BDT", L"BDT AG" },
    { "BECEEM", L"Beceem Communications, Inc" },
    { "BENQ", L"BENQ Corporation." },
    { "BERGSWD", L"Berg Software Design" },
    { "BEZIER", L"Bezier Systems, Inc." },
    { "BHTi", L"Breece Hill Technologies" },
    { "BIOS", L"BIOS Corporation" },
    { "BIR", L"Bio-Imaging Research, Inc." },
    { "BiT", L"BiT Microsystems (obsolete, new ID: BITMICRO)" },
    { "BITMICRO", L"BiT Microsystems, Inc." },
    { "BLOOMBAS", L"Bloombase Technologies Limited" },
    { "BlueArc", L"BlueArc Corporation" },
    { "BNCHMARK", L"Benchmark Tape Systems Corporation" },
    { "Bosch", L"Robert Bosch GmbH" },
    { "Botman", L"Botmanfamily Electronics" },
    { "BoxHill", L"Box Hill Systems Corporation  (Obs. - now Dot Hill)" },
    { "BRDGWRKS", L"Bridgeworks Ltd." },
    { "BREA", L"BREA Technologies, Inc." },
    { "BREECE", L"Breece Hill LLC" },
    { "Broadcom", L"Broadcom Corporation" },
    { "BROCADE", L"Brocade Communications Systems, Incorporated" },
    { "BUFFALO", L"BUFFALO INC." },
    { "BULL", L"Bull Peripherals Corp." },
    { "BUSLOGIC", L"BusLogic Inc." },
    { "BVIRTUAL", L"B-Virtual N.V." },
    { "CalComp", L"CalComp, A Lockheed Company" },
    { "CALCULEX", L"CALCULEX, Inc." },
    { "CALIPER", L"Caliper (California Peripheral Corp.)" },
    { "CAMBEX", L"Cambex Corporation" },
    { "CAMEOSYS", L"Cameo Systems Inc." },
    { "CANDERA", L"Candera Inc." },
    { "CAPTION", L"CAPTION BANK" },
    { "CAST", L"Advanced Storage Tech" },
    { "CATALYST", L"Catalyst Enterprises" },
    { "CCDISK", L"iSCSI Cake" },
    { "CDC", L"Control Data or MPI" },
    { "CDP", L"Columbia Data Products" },
    { "Celsia", L"A M Bromley Limited" },
    { "CenData", L"Central Data Corporation" },
    { "Cereva", L"Cereva Networks Inc." },
    { "CERTANCE", L"Certance" },
    { "CHEROKEE", L"Cherokee Data Systems" },
    { "CHINON", L"Chinon" },
    { "CHRISTMA", L"Christmann Informationstechnik + Medien GmbH & Co KG" },
    { "CIE&YED", L"YE Data, C.Itoh Electric Corp." },
    { "CIPHER", L"Cipher Data Products" },
    { "Ciprico", L"Ciprico, Inc." },
    { "CIRRUSL", L"Cirrus Logic Inc." },
    { "CISCO", L"Cisco Systems, Inc." },
    { "CLOVERLF", L"Cloverleaf Communications, Inc" },
    { "CLS", L"Celestica" },
    { "CMD", L"CMD Technology Inc." },
    { "CMTechno", L"CMTech" },
    { "CNGR SFW", L"Congruent Software, Inc." },
    { "CNSi", L"Chaparral Network Storage, Inc." },
    { "CNT", L"Computer Network Technology" },
    { "COBY", L"Coby Electronics Corporation, USA" },
    { "COGITO", L"Cogito" },
    { "COMPAQ", L"Compaq Computer Corporation (now HP)" },
    { "COMPELNT", L"Compellent Technologies, Inc. (now Dell)" },
    { "COMPORT", L"Comport Corp." },
    { "COMPSIG", L"Computer Signal Corporation" },
    { "COMPTEX", L"Comptex Pty Limited" },
    { "CONNER", L"Conner Peripherals" },
    { "COPANSYS", L"COPAN SYSTEMS INC" },
    { "CORAID", L"Coraid, Inc" },
    { "CORE", L"Core International, Inc." },
    { "COVOTE", L"Covote GmbH & Co KG" },
    { "COWON", L"COWON SYSTEMS, Inc." },
    { "CPL", L"Cross Products Ltd" },
    { "CPU TECH", L"CPU Technology, Inc." },
    { "CREO", L"Creo Products Inc." },
    { "CROSFLD", L"Crosfield Electronics (now FujiFilm Electonic Imaging Ltd)" },
    { "CROSSRDS", L"Crossroads Systems, Inc." },
    { "crosswlk", L"Crosswalk, Inc." },
    { "CSCOVRTS", L"Cisco - Veritas" },
    { "CSM, INC", L"Computer SM, Inc." },
    { "Cunuqui", L"CUNUQUI SLU" },
    { "CYBERNET", L"Cybernetics" },
    { "Cygnal", L"Dekimo" },
    { "D Bit", L"Digby's Bitpile, Inc. DBA D Bit" },
    { "DALSEMI", L"Dallas Semiconductor" },
    { "DANEELEC", L"Dane-Elec" },
    { "DANGER", L"Danger Inc." },
    { "DAT-MG", L"DAT Manufacturers Group" },
    { "Data Com", L"Data Com Information Systems Pty. Ltd." },
    { "DATABOOK", L"Databook, Inc." },
    { "DATACOPY", L"Datacopy Corp." },
    { "DataCore", L"DataCore Software Corporation" },
    { "DATAPT", L"Datapoint Corp." },
    { "DATARAM", L"Dataram Corporation" },
    { "DAVIS", L"Daviscomms (S) Pte Ltd" },
    { "DDN", L"DataDirect Networks, Inc." },
    { "DDRDRIVE", L"DDRdrive LLC" },
    { "DE", L"Dimension Engineering LLC" },
    { "DEC", L"Digital Equipment Corporation (now HP)" },
    { "DEI", L"Digital Engineering, Inc." },
    { "DELL", L"Dell, Inc." },
    { "DELPHI", L"Delphi Data Div. of Sparks Industries, Inc." },
    { "DENON", L"Denon/Nippon Columbia" },
    { "DenOptix", L"DenOptix, Inc." },
    { "DEST", L"DEST Corp." },
    { "DFC", L"DavioFranke.com" },
    { "DGC", L"Data General Corp." },
    { "DIGIDATA", L"Digi-Data Corporation" },
    { "DigiIntl", L"Digi International" },
    { "Digital", L"Digital Equipment Corporation (now HP)" },
    { "DILOG", L"Distributed Logic Corp." },
    { "DISC", L"Document Imaging Systems Corp." },
    { "DLNET", L"Driveline" },
    { "DNS", L"Data and Network Security" },
    { "DNUK", L"Digital Networks Uk Ltd" },
    { "DotHill", L"Dot Hill Systems Corp." },
    { "DP", L"Dell, Inc." },
    { "DPT", L"Distributed Processing Technology" },
    { "DROBO", L"Data Robotics, Inc." },
    { "DSC", L"DigitalStream Corporation" },
    { "DSI", L"Data Spectrum, Inc." },
    { "DSM", L"Deterner Steuerungs- und Maschinenbau GmbH & Co." },
    { "DSNET", L"Cleversafe, Inc." },
    { "DT", L"Double-Take Software, INC." },
    { "DTC QUME", L"Data Technology Qume" },
    { "DXIMAGIN", L"DX Imaging" },
    { "EARTHLAB", L"EarthLabs" },
    { "EarthLCD", L"Earth Computer Technologies, Inc." },
    { "ECCS", L"ECCS, Inc." },
    { "ECMA", L"European Computer Manufacturers Association" },
    { "EDS", L"Embedded Data Systems" },
    { "ELE Intl", L"ELE International" },
    { "ELEGANT", L"Elegant Invention, LLC" },
    { "Elektron", L"Elektron Music Machines MAV AB" },
    { "elipsan", L"Elipsan UK Ltd." },
    { "Elms", L"Elms Systems Corporation" },
    { "ELSE", L"ELSE Ltd." },
    { "ELSEC", L"Littlemore Scientific" },
    { "EMASS", L"EMASS, Inc." },
    { "EMC", L"EMC Corp." },
    { "EMiT", L"EMiT Conception Eletronique" },
    { "EMTEC", L"EMTEC Magnetics" },
    { "EMULEX", L"Emulex" },
    { "ENERGY-B", L"Energybeam Corporation" },
    { "ENGENIO", L"Engenio Information Technologies, Inc." },
    { "ENMOTUS", L"Enmotus Inc" },
    { "EPOS", L"EPOS Technologies Ltd." },
    { "EPSON", L"Epson" },
    { "EQLOGIC", L"EqualLogic" },
    { "Eris/RSI", L"RSI Systems, Inc." },
    { "ETERNE", L"EterneData Technology Co.,Ltd.(China PRC.)" },
    { "EuroLogc", L"Eurologic Systems Limited (now part of Adaptec)" },
    { "evolve", L"Evolution Technologies, Inc" },
    { "EXABYTE", L"Exabyte Corp." },
    { "EXATEL", L"Exatelecom Co., Ltd." },
    { "EXAVIO", L"Exavio, Inc." },
    { "Exsequi", L"Exsequi Ltd" },
    { "Exxotest", L"Annecy Electronique" },
    { "FAIRHAVN", L"Fairhaven Health, LLC" },
    { "FALCON", L"FalconStor, Inc." },
    { "FFEILTD", L"FujiFilm Electonic Imaging Ltd" },
    { "Fibxn", L"Fiberxon, Inc." },
    { "FID", L"First International Digital, Inc." },
    { "FILENET", L"FileNet Corp." },
    { "FirmFact", L"Firmware Factory Ltd" },
    { "FLYFISH", L"Flyfish Technologies" },
    { "FOXCONN", L"Foxconn Technology Group" },
    { "FRAMDRV", L"FRAMEDRIVE Corp." },
    { "FREECION", L"Nable Communications, Inc." },
    { "FSC", L"Fujitsu Siemens Computers" },
    { "FTPL", L"Frontline Technologies Pte Ltd" },
    { "FUJI", L"Fuji Electric Co., Ltd. (Japan)" },
    { "FUJIFILM", L"Fuji Photo Film, Co., Ltd." },
    { "FUJITSU", L"Fujitsu" },
    { "FUNAI", L"Funai Electric Co., Ltd." },
    { "FUSIONIO", L"Fusion-io Inc." },
    { "FUTURED", L"Future Domain Corp." },
    { "G&D", L"Giesecke & Devrient GmbH" },
    { "G.TRONIC", L"Globaltronic - Electronica e Telecomunicacoes, S.A." },
    { "Gadzoox", L"Gadzoox Networks, Inc. (now part of Broadcom)" },
    { "Gammaflx", L"Gammaflux L.P." },
    { "GDI", L"Generic Distribution International" },
    { "GEMALTO", L"gemalto" },
    { "Gen_Dyn", L"General Dynamics" },
    { "Generic", L"Generic Technology Co., Ltd." },
    { "GENSIG", L"General Signal Networks" },
    { "GIGATAPE", L"GIGATAPE GmbH" },
    { "GIGATRND", L"GigaTrend Incorporated" },
    { "Global", L"Global Memory Test Consortium" },
    { "Gnutek", L"Gnutek Ltd." },
    { "Goidelic", L"Goidelic Precision, Inc." },
    { "GoldKey", L"GoldKey Security Corporation" },
    { "GoldStar", L"LG Electronics Inc." },
    { "GORDIUS", L"Gordius" },
    { "GOULD", L"Gould" },
    { "HAGIWARA", L"Hagiwara Sys-Com Co., Ltd." },
    { "HAPP3", L"Inventec Multimedia and Telecom co., ltd" },
    { "HDS", L"Horizon Data Systems, Inc." },
    { "Heydays", L"Mazo Technology Co., Ltd." },
    { "HI-TECH", L"HI-TECH Software Pty. Ltd." },
    { "HITACHI", L"Hitachi America Ltd or Nissei Sangyo America Ltd" },
    { "HL-DT-ST", L"Hitachi-LG Data Storage, Inc." },
    { "HONEYWEL", L"Honeywell Inc." },
    { "Hoptroff", L"HexWax Ltd" },
    { "HORIZONT", L"Horizontigo Software" },
    { "HP", L"Hewlett Packard" },
    { "HPQ", L"Hewlett Packard" },
    { "HUASY", L"Huawei Symantec Technologies Co., Ltd." },
    { "HYUNWON", L"HYUNWON inc" },
    { "i-cubed", L"i-cubed ltd." },
    { "IBM", L"International Business Machines" },
    { "Icefield", L"Icefield Tools Corporation" },
    { "ICL", L"ICL" },
    { "ICP", L"ICP vortex Computersysteme GmbH" },
    { "IDE", L"International Data Engineering, Inc." },
    { "IDG", L"Interface Design Group" },
    { "IET", L"ISCSI ENTERPRISE TARGET" },
    { "IFT", L"Infortrend Technology, Inc." },
    { "IGR", L"Intergraph Corp." },
    { "IMATION", L"Imation" },
    { "IMPLTD", L"Integrated Micro Products Ltd." },
    { "IMPRIMIS", L"Imprimis Technology Inc." },
    { "INCIPNT", L"Incipient Technologies Inc." },
    { "INCITS", L"InterNational Committee for Information Technology" },
    { "INDCOMP", L"Industrial Computing Limited" },
    { "Indigita", L"Indigita Corporation" },
    { "INITIO", L"Initio Corporation" },
    { "INRANGE", L"INRANGE Technologies Corporation" },
    { "Insight", L"L-3 Insight Technology Inc" },
    { "INSITE", L"Insite Peripherals" },
    { "integrix", L"Integrix, Inc." },
    { "INTEL", L"Intel Corporation" },
    { "Intransa", L"Intransa, Inc." },
    { "IOC", L"I/O Concepts, Inc." },
    { "iofy", L"iofy Corporation" },
    { "IOMEGA", L"Iomega" },
    { "IOT", L"IO Turbine, Inc." },
    { "iqstor", L"iQstor Networks, Inc." },
    { "iQue", L"iQue" },
    { "ISi", L"Information Storage inc." },
    { "Isilon", L"Isilon Systems, Inc." },
    { "ISO", L"International Standards Organization" },
    { "iStor", L"iStor Networks, Inc." },
    { "ITC", L"International Tapetronics Corporation" },
    { "iTwin", L"iTwin Pte Ltd" },
    { "IVIVITY", L"iVivity, Inc." },
    { "IVMMLTD", L"InnoVISION Multimedia Ltd." },
    { "JABIL001", L"Jabil Circuit" },
    { "JETWAY", L"Jetway Information Co., Ltd" },
    { "JMR", L"JMR Electronics Inc." },
    { "JOLLYLOG", L"Jolly Logic" },
    { "JPC Inc.", L"JPC Inc." },
    { "JSCSI", L"jSCSI Project" },
    { "Juniper", L"Juniper Networks" },
    { "JVC", L"JVC Information Products Co." },
    { "KASHYA", L"Kashya, Inc." },
    { "KENNEDY", L"Kennedy Company" },
    { "KENWOOD", L"KENWOOD Corporation" },
    { "KEWL", L"Shanghai KEWL Imp&Exp Co., Ltd." },
    { "Key Tech", L"Key Technologies, Inc" },
    { "KMNRIO", L"Kaminario Technologies Ltd." },
    { "KODAK", L"Eastman Kodak" },
    { "KONAN", L"Konan" },
    { "koncepts", L"koncepts International Ltd." },
    { "KONICA", L"Konica Japan" },
    { "KOVE", L"KOVE" },
    { "KSCOM", L"KSCOM Co. Ltd." },
    { "KUDELSKI", L"Nagravision SA - Kudelski Group" },
    { "Kyocera", L"Kyocera Corporation" },
    { "Lapida", L"Gonmalo Electronics" },
    { "LAPINE", L"Lapine Technology" },
    { "LASERDRV", L"LaserDrive Limited" },
    { "LASERGR", L"Lasergraphics, Inc." },
    { "LeapFrog", L"LeapFrog Enterprises, Inc." },
    { "LEFTHAND", L"LeftHand Networks (now HP)" },
    { "Leica", L"Leica Camera AG" },
    { "Lexar", L"Lexar Media, Inc." },
    { "LEYIO", L"LEYIO" },
    { "LG", L"LG Electronics Inc." },
    { "LGE", L"LG Electronics Inc." },
    { "LION", L"Lion Optics Corporation" },
    { "LMS", L"Laser Magnetic Storage International Company" },
    { "LoupTech", L"Loup Technologies, Inc." },
    { "LSI", L"LSI Corp. (was LSI Logic Corp.)" },
    { "LSILOGIC", L"LSI Logic Storage Systems, Inc." },
    { "LTO-CVE", L"Linear Tape - Open, Compliance Verification Entity" },
    { "LUXPRO", L"Luxpro Corporation" },
    { "Malakite", L"Malachite Technologies (New VID is: Sandial)" },
    { "MarcBoon", L"marcboon.com" },
    { "Marner", L"Marner Storage Technologies, Inc." },
    { "MARVELL", L"Marvell Semiconductor, Inc." },
    { "MATSHITA", L"Matsushita" },
    { "MAXELL", L"Hitachi Maxell, Ltd." },
    { "MAXIM-IC", L"Maxim Integrated Products" },
    { "MaxOptix", L"Maxoptix Corp." },
    { "MAXSTRAT", L"Maximum Strategy, Inc." },
    { "MAXTOR", L"Maxtor Corp." },
    { "MaXXan", L"MaXXan Systems, Inc." },
    { "MAYCOM", L"maycom Co., Ltd." },
    { "MBEAT", L"K-WON C&C Co.,Ltd" },
    { "MCC", L"Measurement Computing Corporation" },
    { "McDATA", L"McDATA Corporation" },
    { "MCUBE", L"Mcube Technology Co., Ltd." },
    { "MDI", L"Micro Design International, Inc." },
    { "MEADE", L"Meade Instruments Corporation" },
    { "mediamat", L"mediamatic" },
    { "MEII", L"Mountain Engineering II, Inc." },
    { "MELA", L"Mitsubishi Electronics America" },
    { "MELCO", L"Mitsubishi Electric (Japan)" },
    { "mellanox", L"Mellanox Technologies Ltd." },
    { "MEMOREX", L"Memorex Telex Japan Ltd." },
    { "MEMREL", L"Memrel Corporation" },
    { "MEMTECH", L"MemTech Technology" },
    { "MendoCno", L"Mendocino Software" },
    { "MERIDATA", L"Oy Meridata Finland Ltd" },
    { "METHODEI", L"Methode Electronics India pvt ltd" },
    { "METRUM", L"Metrum, Inc." },
    { "MHTL", L"Matsunichi Hi-Tech Limited" },
    { "MICROBTX", L"Microbotics Inc." },
    { "Microchp", L"Microchip Technology, Inc." },
    { "MICROLIT", L"Microlite Corporation" },
    { "MICRON", L"Micron Technology, Inc." },
    { "MICROP", L"Micropolis" },
    { "MICROTEK", L"Microtek Storage Corp" },
    { "Minitech", L"Minitech (UK) Limited" },
    { "Minolta", L"Minolta Corporation" },
    { "MINSCRIB", L"Miniscribe" },
    { "MiraLink", L"MiraLink Corporation" },
    { "Mirifica", L"Mirifica s.r.l." },
    { "MITSUMI", L"Mitsumi Electric Co., Ltd." },
    { "MKM", L"Mitsubishi Kagaku Media Co., LTD." },
    { "Mobii", L"Mobii Systems (Pty.) Ltd." },
    { "MOL", L"Petrosoft Sdn. Bhd." },
    { "MOSAID", L"Mosaid Technologies Inc." },
    { "MOTOROLA", L"Motorola" },
    { "MP-400", L"Daiwa Manufacturing Limited" },
    { "MPC", L"MPC Corporation" },
    { "MPCCORP", L"MPC Computers" },
    { "MPEYE", L"Touchstone Technology Co., Ltd" },
    { "MPIO", L"DKT Co.,Ltd" },
    { "MPM", L"Mitsubishi Paper Mills, Ltd." },
    { "MPMan", L"MPMan.com, Inc." },
    { "MSFT", L"Microsoft Corporation" },
    { "MSI", L"Micro-Star International Corp." },
    { "MST", L"Morning Star Technologies, Inc." },
    { "MSystems", L"M-Systems Flash Disk Pioneers" },
    { "MTI", L"MTI Technology Corporation" },
    { "MTNGATE", L"MountainGate Data Systems" },
    { "MXI", L"Memory Experts International" },
    { "nac", L"nac Image Technology Inc." },
    { "NAGRA", L"Nagravision SA - Kudelski Group" },
    { "NAI", L"North Atlantic Industries" },
    { "NAKAMICH", L"Nakamichi Corporation" },
    { "NatInst", L"National Instruments" },
    { "NatSemi", L"National Semiconductor Corp." },
    { "NCITS", L"InterNational Committee for Information Technology Standards (INCITS)" },
    { "NCL", L"NCL America" },
    { "NCR", L"NCR Corporation" },
    { "Neartek", L"Neartek, Inc." },
    { "NEC", L"NEC" },
    { "NETAPP", L"NetApp, Inc. (was Network Appliance)" },
    { "NetBSD", L"The NetBSD Foundation" },
    { "Netcom", L"Netcom Storage" },
    { "NETENGIN", L"NetEngine, Inc." },
    { "NEWISYS", L"Newisys Data Storage" },
    { "Newtech", L"Newtech Co., Ltd." },
    { "NEXSAN", L"Nexsan Technologies, Ltd." },
    { "NFINIDAT", L"Infinidat Ltd." },
    { "NHR", L"NH Research, Inc." },
    { "Nike", L"Nike, Inc." },
    { "Nimble", L"Nimble Storage" },
    { "NISCA", L"NISCA Inc." },
    { "NISHAN", L"Nishan Systems Inc." },
    { "NKK", L"NKK Corp." },
    { "NRC", L"Nakamichi Research Corporation" },
    { "NSD", L"Nippon Systems Development Co.,Ltd." },
    { "NSM", L"NSM Jukebox GmbH" },
    { "nStor", L"nStor Technologies, Inc." },
    { "NT", L"Northern Telecom" },
    { "NUCONNEX", L"NuConnex" },
    { "NUSPEED", L"NuSpeed, Inc." },
    { "NVIDIA", L"NVIDIA Corporation" },
    { "NVMe", L"NVM Express Working Group" },
    { "OAI", L"Optical Access International" },
    { "OCE", L"Oce Graphics" },
    { "OHDEN", L"Ohden Co., Ltd." },
    { "OKI", L"OKI Electric Industry Co.,Ltd (Japan)" },
    { "Olidata", L"Olidata S.p.A." },
    { "OMI", L"Optical Media International" },
    { "OMNIFI", L"Rockford Corporation - Omnifi Media" },
    { "OMNIS", L"OMNIS Company (FRANCE)" },
    { "Ophidian", L"Ophidian Designs" },
    { "opslag", L"Tyrone Systems" },
    { "Optelec", L"Optelec BV" },
    { "Optiarc", L"Sony Optiarc Inc." },
    { "OPTIMEM", L"Cipher/Optimem" },
    { "OPTOTECH", L"Optotech" },
    { "ORACLE", L"Oracle Corporation" },
    { "ORANGE", L"Orange Micro, Inc." },
    { "ORCA", L"Orca Technology" },
    { "OSI", L"Optical Storage International" },
    { "OSNEXUS", L"OS NEXUS, Inc." },
    { "OTL", L"OTL Engineering" },
    { "OVERLAND", L"Overland Storage Inc." },
    { "pacdigit", L"Pacific Digital Corp" },
    { "Packard", L"Parkard Bell" },
    { "Panasas", L"Panasas, Inc." },
    { "PARALAN", L"Paralan Corporation" },
    { "PASCOsci", L"Pasco Scientific" },
    { "PATHLGHT", L"Pathlight Technology, Inc." },
    { "PerStor", L"Perstor" },
    { "PERTEC", L"Pertec Peripherals Corporation" },
    { "PFTI", L"Performance Technology Inc." },
    { "PFU", L"PFU Limited" },
    { "PHILIPS", L"Philips Electronics" },
    { "PICO", L"Packard Instrument Company" },
    { "PIK", L"TECHNILIENT & MCS" },
    { "Pillar", L"Pillar Data Systems" },
    { "PIONEER", L"Pioneer Electronic Corp." },
    { "Pirus", L"Pirus Networks" },
    { "PIVOT3", L"Pivot3, Inc." },
    { "PLASMON", L"Plasmon Data" },
    { "Pliant", L"Pliant Technology, Inc." },
    { "PMCSIERA", L"PMC-Sierra" },
    { "PNNMed", L"PNN Medical SA" },
    { "POKEN", L"Poken SA" },
    { "POLYTRON", L"PT. HARTONO ISTANA TEKNOLOGI" },
    { "PRAIRIE", L"PrairieTek" },
    { "PREPRESS", L"PrePRESS Solutions" },
    { "PRESOFT", L"PreSoft Architects" },
    { "PRESTON", L"Preston Scientific" },
    { "PRIAM", L"Priam" },
    { "PRIMAGFX", L"Primagraphics Ltd" },
    { "PRIMOS", L"Primos" },
    { "PROCOM", L"Procom Technology" },
    { "PROLIFIC", L"Prolific Technology Inc." },
    { "PROMISE", L"PROMISE TECHNOLOGY, Inc" },
    { "PROSTOR", L"ProStor Systems, Inc." },
    { "PROSUM", L"PROSUM" },
    { "PROWARE", L"Proware Technology Corp." },
    { "PTI", L"Peripheral Technology Inc." },
    { "PTICO", L"Pacific Technology International" },
    { "PURE", L"PURE Storage" },
    { "QIC", L"Quarter-Inch Cartridge Drive Standards, Inc." },
    { "QLogic", L"QLogic Corporation" },
    { "QNAP", L"QNAP Systems" },
    { "Qsan", L"QSAN Technology, Inc." },
    { "QUALSTAR", L"Qualstar" },
    { "QUANTEL", L"Quantel Ltd." },
    { "QUANTUM", L"Quantum Corp." },
    { "QUIX", L"Quix Computerware AG" },
    { "R-BYTE", L"R-Byte, Inc." },
    { "RACALREC", L"Racal Recorders" },
    { "RADITEC", L"Radikal Technologies Deutschland GmbH" },
    { "RADSTONE", L"Radstone Technology" },
    { "RASSYS", L"Rasilient Systems Inc." },
    { "RASVIA", L"Rasvia Systems, Inc." },
    { "rave-mp", L"Go Video" },
    { "Readboy", L"Readboy Ltd Co." },
    { "Realm", L"Realm Systems" },
    { "realtek", L"Realtek Semiconductor Corp." },
    { "RELDATA", L"RELDATA Inc" },
    { "RENAGmbH", L"RENA GmbH" },
    { "Revivio", L"Revivio, Inc." },
    { "RGI", L"Raster Graphics, Inc." },
    { "RHAPSODY", L"Rhapsody Networks, Inc." },
    { "RHS", L"Racal-Heim Systems GmbH" },
    { "RICOH", L"Ricoh" },
    { "RODIME", L"Rodime" },
    { "Royaltek", L"RoyalTek company Ltd." },
    { "RPS", L"RPS" },
    { "RTI", L"Reference Technology" },
    { "S-D", L"Sauer-Danfoss" },
    { "S-flex", L"Storageflex Inc" },
    { "S-SYSTEM", L"S-SYSTEM" },
    { "S1", L"storONE" },
    { "SAMSUNG", L"Samsung Electronics Co., Ltd." },
    { "SAN", L"Storage Area Networks, Ltd." },
    { "Sandial", L"Sandial Systems, Inc." },
    { "SanDisk", L"SanDisk Corporation" },
    { "SANKYO", L"Sankyo Seiki" },
    { "SANRAD", L"SANRAD Inc." },
    { "SANYO", L"SANYO Electric Co., Ltd." },
    { "SC.Net", L"StorageConnections.Net" },
    { "SCALE", L"Scale Computing, Inc." },
    { "SCIENTEK", L"SCIENTEK CORP" },
    { "SCInc.", L"Storage Concepts, Inc." },
    { "SCREEN", L"Dainippon Screen Mfg. Co., Ltd." },
    { "SDI", L"Storage Dimensions, Inc." },
    { "SDS", L"Solid Data Systems" },
    { "SEAC", L"SeaChange International, Inc." },
    { "SEAGATE", L"Seagate" },
    { "SEAGRAND", L"SEAGRAND In Japan" },
    { "Seanodes", L"Seanodes" },
    { "Sec. Key", L"SecureKey Technologies Inc." },
    { "SEQUOIA", L"Sequoia Advanced Technologies, Inc." },
    { "Shinko", L"Shinko Electric Co., Ltd." },
    { "SIEMENS", L"Siemens" },
    { "SigmaTel", L"SigmaTel, Inc." },
    { "SII", L"Seiko Instruments Inc." },
    { "SIMPLE", L"SimpleTech, Inc. (Obs - now STEC, Inc.)" },
    { "SLCNSTOR", L"SiliconStor, Inc." },
    { "SLI", L"Sierra Logic, Inc." },
    { "SMS", L"Scientific Micro Systems/OMTI" },
    { "SMSC", L"SMSC Storage, Inc." },
    { "SMX", L"Smartronix, Inc." },
    { "SNYSIDE", L"Sunnyside Computing Inc." },
    { "SoftLock", L"Softlock Digital Security Provider" },
    { "SolidFir", L"SolidFire, Inc." },
    { "SONIC", L"Sonic Solutions" },
    { "SoniqCas", L"SoniqCast" },
    { "SONY", L"Sony Corporation Japan" },
    { "SOUL", L"Soul Storage Technology (Wuxi) Co., Ltd" },
    { "SPD", L"Storage Products Distribution, Inc." },
    { "SPECIAL", L"Special Computing Co." },
    { "SPECTRA", L"Spectra Logic, a Division of Western Automation Labs, Inc." },
    { "SPERRY", L"Sperry (now Unisys Corp.)" },
    { "Spintso", L"Spintso International AB" },
    { "STARWIND", L"StarWind Software, Inc." },
    { "STEC", L"STEC, Inc." },
    { "Sterling", L"Sterling Diagnostic Imaging, Inc." },
    { "STK", L"Storage Technology Corporation" },
    { "STNWOOD", L"Stonewood Group" },
    { "STONEFLY", L"StoneFly Networks, Inc." },
    { "STOR", L"StorageNetworks, Inc." },
    { "STORAPPVStorageApps, Inc. (now HP)" },
    { "STORCOMP", L"Storage Computer Corporation" },
    { "STORM", L"Storm Technology, Inc." },
    { "StorMagc", L"StorMagic" },
    { "Stratus", L"Stratus Technologies" },
    { "StrmLgc", L"StreamLogic Corp." },
    { "SUMITOMO", L"Sumitomo Electric Industries, Ltd." },
    { "SUN", L"Sun Microsystems, Inc." },
    { "SUNCORP", L"SunCorporation" },
    { "suntx", L"Suntx System Co., Ltd" },
    { "Swinxs", L"Swinxs BV" },
    { "SYMANTEC", L"Symantec Corporation" },
    { "SYMBIOS", L"Symbios Logic Inc." },
    { "SYMWAVE", L"Symwave, Inc." },
    { "SYNCSORT", L"Syncsort Incorporated" },
    { "SYNERWAY", L"Synerway" },
    { "SYNOLOGY", L"Synology, Inc." },
    { "SyQuest", L"SyQuest Technology, Inc." },
    { "SYSGEN", L"Sysgen" },
    { "T-MITTON", L"Transmitton England" },
    { "T-MOBILE", L"T-Mobile USA, Inc." },
    { "T11", L"INCITS Technical Committee T11" },
    { "TALARIS", L"Talaris Systems, Inc." },
    { "TALLGRAS", L"Tallgrass Technologies" },
    { "TANDBERG", L"Tandberg Data A/S" },
    { "TANDEM", L"Tandem (now HP)" },
    { "TANDON", L"Tandon" },
    { "TCL", L"TCL Shenzhen ASIC MIcro-electronics Ltd" },
    { "TDK", L"TDK Corporation" },
    { "TEAC", L"TEAC Japan" },
    { "TECOLOTE", L"Tecolote Designs" },
    { "TEGRA", L"Tegra Varityper" },
    { "Tek", L"Tektronix" },
    { "TELLERT", L"Tellert Elektronik GmbH" },
    { "TENTIME", L"Laura Technologies, Inc." },
    { "TFDATACO", L"TimeForge" },
    { "TGEGROUP", L"TGE Group Co.,LTD." },
    { "Thecus", L"Thecus Technology Corp." },
    { "TI-DSG", L"Texas Instruments" },
    { "TiGi", L"TiGi Corporation" },
    { "TILDESGN", L"Tildesign bv" },
    { "Tite", L"Tite Technology Limited" },
    { "TKS Inc.", L"TimeKeeping Systems, Inc." },
    { "TMS", L"Texas Memory Systems, Inc." },
    { "TMS100", L"TechnoVas" },
    { "TOLISGRP", L"The TOLIS Group" },
    { "TOSHIBA", L"Toshiba Japan" },
    { "TRIOFLEX", L"Trioflex Oy" },
    { "TRIPACE", L"Tripace" },
    { "TRLogger", L"TrueLogger Ltd." },
    { "TROIKA", L"Troika Networks, Inc." },
    { "TRULY", L"TRULY Electronics MFG. LTD." },
    { "TRUSTED", L"Trusted Data Corporation" },
    { "TSSTcorp", L"Toshiba Samsung Storage Technology Corporation" },
    { "TZM", L"TZ Medical" },
    { "UD-DVR", L"Bigstone Project." },
    { "UDIGITAL", L"United Digital Limited" },
    { "UIT", L"United Infomation Technology" },
    { "ULTRA", L"UltraStor Corporation" },
    { "UNISTOR", L"Unistor Networks, Inc." },
    { "UNISYS", L"Unisys" },
    { "USCORE", L"Underscore, Inc." },
    { "USDC", L"US Design Corp." },
    { "VASCO", L"Vasco Data Security" },
    { "VDS", L"Victor Data Systems Co., Ltd." },
    { "Verari", L"Verari Systems, Inc." },
    { "VERBATIM", L"Verbatim Corporation" },
    { "Vercet", L"Vercet LLC" },
    { "VERITAS", L"VERITAS Software Corporation" },
    { "VEXCEL", L"VEXCEL IMAGING GmbH" },
    { "VicomSys", L"Vicom Systems, Inc." },
    { "VIDEXINC", L"Videx, Inc." },
    { "VIOLIN", L"Violin Memory, Inc." },
    { "VITESSE", L"Vitesse Semiconductor Corporation" },
    { "VIXEL", L"Vixel Corporation (now part of Emulex)" },
    { "VLS", L"Van Lent Systems BV" },
    { "VMAX", L"VMAX Technologies Corp." },
    { "VMware", L"VMware Inc." },
    { "Vobis", L"Vobis Microcomputer AG" },
    { "VOLTAIRE", L"Voltaire Ltd." },
    { "VRC", L"Vermont Research Corp." },
    { "VRugged", L"Vanguard Rugged Storage" },
    { "Waitec", L"Waitec NV" },
    { "WangDAT", L"WangDAT" },
    { "WANGTEK", L"Wangtek" },
    { "Wasabi", L"Wasabi Systems" },
    { "WAVECOM", L"Wavecom" },
    { "WD", L"Western Digital Corporation" },
    { "WDC", L"Western Digital Corporation" },
    { "WDIGTL", L"Western Digital" },
    { "WEARNES", L"Wearnes Technology Corporation" },
    { "WeeraRes", L"Weera Research Pte Ltd" },
    { "Wildflwr", L"Wildflower Technologies, Inc." },
    { "WSC0001", L"Wisecom, Inc." },
    { "X3", L"InterNational Committee for Information Technology Standards (INCITS)" },
    { "XEBEC", L"Xebec Corporation" },
    { "XENSRC", L"XenSource, Inc." },
    { "Xerox", L"Xerox Corporation" },
    { "XIOtech", L"XIOtech Corporation" },
    { "XIRANET", L"Xiranet Communications GmbH" },
    { "XIV", L"XIV (now IBM)" },
    { "XtremIO", L"XtremIO" },
    { "XYRATEX", L"Xyratex" },
    { "YINHE", L"NUDT Computer Co." },
    { "YIXUN", L"Yixun Electronic Co.,Ltd." },
    { "YOTTA", L"YottaYotta, Inc." },
    { "Zarva", L"Zarva Digital Technology Co., Ltd." },
    { "ZETTA", L"Zetta Systems, Inc." },
    { 0 }
};

VOID
GetScsiVendorById(LPSTR lpVendorId, LPWSTR lpVendorName, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpVendorName[0] = 0;

    do
    {
        if (strcmp(ScsiVendors[Index].pVendorId, lpVendorId) == 0)
        {
            StringCbCopy(lpVendorName, Size, ScsiVendors[Index].pVendorName);
            return;
        }
    }
    while (ScsiVendors[++Index].pVendorId != 0);
}

BOOL
IsFeatureSupported(HANDLE hHandle, WORD wFeatureCode)
{
    SCSI_GET_CONFIG Config;
    BOOL IsSupported = FALSE;

    if (GetConfinurationScsi(hHandle,
                             wFeatureCode,
                             &Config))
    {
        if ((((Config.Feature.featureCode << 8) & 0xFF00) |
            ((Config.Feature.featureCode >> 8) & 0x00FF)) == wFeatureCode)
        {
            IsSupported = TRUE;
        }
    }

    return IsSupported;
}

VOID
GetReadWriteFeature(LPWSTR lpName, BYTE bRead, BYTE bWrite)
{
    WCHAR szText[MAX_STR_LEN];
    UINT id;

    IoAddItem(2, 1, lpName);

    if (bRead && bWrite)
        id = IDS_CDROM_READ_WRITE;
    else if (bRead)
        id = IDS_CDROM_READ;
    else if (bWrite)
        id = IDS_CDROM_WRITE;
    else
        id = IDS_CPUID_UNSUPPORTED;
    LoadMUIString(id, szText, MAX_STR_LEN);
    IoSetItemText(szText);
}

VOID
GetSerialNumber(HANDLE hHandle, LPWSTR lpSerial, SIZE_T SerialSize)
{
    SCSI_GET_CONFIG Config;

    lpSerial[0] = 0;
    if (GetConfinurationScsi(hHandle,
                             0x108,
                             &Config))
    {
        if ((((Config.Feature.featureCode << 8) & 0xFF00) |
            ((Config.Feature.featureCode >> 8) & 0x00FF)) == 0x108)
        {
            Config.Feature.additionalData[Config.Feature.additionalLength + 1] = 0;
            StringCbPrintf(lpSerial, SerialSize, L"%S", Config.Feature.additionalData);
            lpSerial[wcslen(lpSerial) - 1] = 0;
        }
    }
}

BOOL
GetInterfaceType(HANDLE hHandle, LPWSTR lpInterface, SIZE_T Size)
{
    SCSI_GET_CONFIG Config;
    WCHAR *pText;

    lpInterface[0] = 0;

    if (GetConfinurationScsi(hHandle,
                             0x01,
                             &Config))
    {
        if ((((Config.Feature.featureCode << 8) & 0xFF00) |
            ((Config.Feature.featureCode >> 8) & 0x00FF)) == 0x01)
        {
            switch (Config.Feature.additionalData[3])
            {
                case 0: pText = L"Unspecified";      break;
                case 1: pText = L"SCSI";             break;
                case 2: pText = L"ATAPI";            break;
                case 3: pText = L"IEEE 1394 - 1995"; break;
                case 4: pText = L"IEEE 1394A";       break;
                case 5: pText = L"Fibre Channel";    break;
                case 6: pText = L"IEEE 1394B";       break;
                case 7: pText = L"Serial ATAPI";     break;
                case 8: pText = L"USB (1.1 or 2.0)"; break;
                default:
                {
                    DebugTrace(L"Unknown interface type = %d",
                               Config.Feature.additionalData[3]);
                    pText = L"Unknown";
                    break;
                }
            }
            StringCbCopy(lpInterface, Size, pText);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
GetFeatureData(HANDLE hHandle,
               WORD wFeatureCode,
               PSCSI_GET_CONFIG Config)
{
    BOOL IsSupported = FALSE;

    if (GetConfinurationScsi(hHandle,
                             wFeatureCode,
                             Config))
    {
        if (((((*Config).Feature.featureCode << 8) & 0xFF00) |
            (((*Config).Feature.featureCode >> 8) & 0x00FF)) == wFeatureCode)
        {
            IsSupported = TRUE;
        }
    }

    return IsSupported;
}

VOID
FillReadWriteSupport(HANDLE hHandle,
                     SCSI_CD_CAPABILITIES Capabilities,
                     PCD_READ_WRITE_SUPPORT pSupport)
{
    CD_READ_WRITE_SUPPORT rw = {0};
    SCSI_GET_CONFIG Config;
    INT i;

    /* CD-ROM */
    rw.CD_ROM_Read  = IsFeatureSupported(hHandle, FEATURE_CD_READ);
    rw.CD_ROM_Write = FALSE;

    /* CD-R */
    rw.CD_R_Read  = Capabilities.CDR_Read;
    rw.CD_R_Write = Capabilities.CDR_Write;

    /* CD-RW */
    rw.CD_RW_Read  = Capabilities.CDRW_Read;
    rw.CD_RW_Write = Capabilities.CDRW_Write;

    /* DVD-ROM */
    rw.DVD_ROM_Read  = Capabilities.DVDROM_Read;
    rw.DVD_ROM_Write = FALSE;

    /* DVD-RAM */
    rw.DVD_RAM_Read  = Capabilities.DVDRAM_Read;
    rw.DVD_RAM_Write = Capabilities.DVDRAM_Write;

    /* DVD+R */
    if (GetFeatureData(hHandle, FEATURE_DVD_PLUSR, &Config))
    {
        rw.DVD_P_R_Read  = TRUE;
        rw.DVD_P_R_Write = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }

    /* DVD+RW */
    if (GetFeatureData(hHandle, FEATURE_DVD_PLUSRW, &Config))
    {
        rw.DVD_P_RW_Read  = TRUE;
        rw.DVD_P_RW_Write = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }

    /* DVD-R */
    rw.DVD_R_Read  = Capabilities.DVDR_Read;
    rw.DVD_R_Write = Capabilities.DVDR_Write;

    /* HD DVD-RAM */
    if (GetFeatureData(hHandle, FEATURE_HD_DVD_READ, &Config))
    {
        rw.HD_DVD_RAM_Read = (Config.Feature.additionalData[2] == 1) ? TRUE : FALSE;
    }
    if (GetFeatureData(hHandle, FEATURE_HD_DVD_WRITE, &Config))
    {
        rw.HD_DVD_RAM_Write = (Config.Feature.additionalData[2] == 1) ? TRUE : FALSE;
    }

    /* DVD-RW */
    if (GetFeatureData(hHandle, FEATURE_DVD_DASHR_DASHRW_WRITE, &Config))
    {
        rw.DVD_RW_Write = GetBitsBYTE(Config.Feature.additionalData[0], 1, 1) ? TRUE : FALSE;
    }
    if (GetFeatureData(hHandle, FEATURE_PROFILE_LIST, &Config))
    {
        for (i = 0; i < Config.Feature.additionalLength; i++)
        {
            if (Config.Feature.additionalData[i] == 0x13 ||
                Config.Feature.additionalData[i] == 0x14)
            {
                rw.DVD_RW_Read = TRUE;
            }

            /* DVD-RW DL */
            if (Config.Feature.additionalData[i] == 0x17 &&
                IsFeatureSupported(hHandle, FEATURE_RIGID_RESTRICTED_OVERWRITE))
            {
                rw.DVD_RW_DL_Write = TRUE;
            }

            /* BD-ROM */
            if (Config.Feature.additionalData[i] == 0x40)
            {
                rw.BD_ROM_Read = TRUE;
            }

            /* BD-R */
            if (Config.Feature.additionalData[i] == 0x41 ||
                Config.Feature.additionalData[i] == 0x42)
            {
                rw.BD_R_Read  = IsFeatureSupported(hHandle, FEATURE_BD_READ);
                rw.BD_R_Write = IsFeatureSupported(hHandle, FEATURE_BD_WRITE);
            }

            /* BD-RE */
            if (Config.Feature.additionalData[i] == 0x40)
            {
                rw.BD_RE_Read  = IsFeatureSupported(hHandle, FEATURE_BD_READ);
                rw.BD_RE_Write = IsFeatureSupported(hHandle, FEATURE_BD_WRITE);
            }

            /* HD DVD-RW */
            if (Config.Feature.additionalData[i] == 0x53)
            {
                rw.HD_DVD_RW_Read  = rw.HD_DVD_RAM_Read;
                rw.HD_DVD_RW_Write = rw.HD_DVD_RAM_Write;
            }

            /* HD DVD-R DL */
            if (Config.Feature.additionalData[i] == 0x58)
            {
                rw.HD_DVD_R_DL_Read  = rw.HD_DVD_RAM_Read;
                rw.HD_DVD_R_DL_Write = rw.HD_DVD_RAM_Write;
            }

            /* HD DVD-RW DL */
            if (Config.Feature.additionalData[i] == 0x5A)
            {
                rw.HD_DVD_RW_DL_Read  = rw.HD_DVD_RAM_Read;
                rw.HD_DVD_RW_DL_Write = rw.HD_DVD_RAM_Write;
            }
        }
    }

    /* DVD-R DL */
    if (GetFeatureData(hHandle, FEATURE_DVD_DASHR_DASHRW_WRITE, &Config))
    {
        rw.DVD_R_DL_Write = GetBitsBYTE(Config.Feature.additionalData[0], 3, 3) ? TRUE : FALSE;
    }
    if (GetFeatureData(hHandle, FEATURE_DVD_READ, &Config))
    {
        rw.DVD_R_DL_Read = GetBitsBYTE(Config.Feature.additionalData[2], 0, 0) ? TRUE : FALSE;
    }

    /* DVD-RW DL */
    if (GetFeatureData(hHandle, FEATURE_DVD_READ, &Config))
    {
        rw.DVD_RW_DL_Read = GetBitsBYTE(Config.Feature.additionalData[0], 1, 1) ? TRUE : FALSE;
    }

    /* DVD+R DL */
    if (GetFeatureData(hHandle, FEATURE_DVD_PLUSR_DL, &Config))
    {
        rw.DVD_P_R_DL_Read  = TRUE;
        rw.DVD_P_R_DL_Write = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }

    /* DVD+RW DL */
    if (GetFeatureData(hHandle, FEATURE_DVD_PLUSRW_DL, &Config))
    {
        rw.DVD_P_RW_DL_Read  = TRUE;
        rw.DVD_P_RW_DL_Write = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }

    /* HD DVD-ROM */
    if (GetFeatureData(hHandle, FEATURE_HD_DVD_READ, &Config))
    {
        rw.HD_DVD_ROM_Read = TRUE;
    }

    /* HD DVD-R */
    if (GetFeatureData(hHandle, FEATURE_HD_DVD_READ, &Config))
    {
        rw.HD_DVD_R_Read = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }
    if (GetFeatureData(hHandle, FEATURE_HD_DVD_WRITE, &Config))
    {
        rw.HD_DVD_R_Write = (Config.Feature.additionalData[0] == 1) ? TRUE : FALSE;
    }

    CopyMemory(pSupport, &rw, sizeof(CD_READ_WRITE_SUPPORT));
}

VOID
HW_CDInfo(VOID)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN],
          szDrives[MAX_PATH];
    CD_READ_WRITE_SUPPORT rw;
    SCSI_CD_CAPABILITIES Capabilities;
    REPORT_KEY_DATA KeyData;
    SCSI_GET_CONFIG Config;
    BOOL IsSupported;
    WCHAR szText[MAX_STR_LEN];
    INQUIRYDATA Inquiry;
    INT Index;
    HANDLE hHandle;

    DebugStartReceiving();

    IoAddIcon(IDI_CD);
    IoAddIcon(IDI_DISK);
    IoAddIcon(IDI_LOCALES);

    LoadMUIString(IDS_CPUID_SUPPORTED,    szSupported,   MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED,  szUnsupported, MAX_STR_LEN);

    if (!GetLogicalDriveStrings(sizeof(szDrives)/sizeof(WCHAR), szDrives))
        return;

    for (Index = 0; szDrives[Index] != 0; Index += 4)
    {
        WCHAR szDrive[3];
        CHAR szVendor[8 + 1] = {0};
        CHAR szProductId[16 + 1] = {0};
        CHAR szProductRev[4 + 1] = {0};

        if (*(szDrives) > L'Z')
        {
            if (szDrives[Index] <= L'Z') szDrives[Index] += 32;
        }
        else
        {
            if (szDrives[Index] > L'Z') szDrives[Index] -= 32;
        }

        StringCbPrintf(szDrive, sizeof(szDrive), L"%c:", szDrives[Index]);

        if (GetDriveType(szDrive) != DRIVE_CDROM)
            continue;

        hHandle = OpenScsiByDriveLetter(szDrives[Index]);

        if (hHandle == INVALID_HANDLE_VALUE)
            continue;

        DebugTrace(L"CdRom (%c:) found!", szDrives[Index]);

        if (!GetInquiryScsi(hHandle, &Inquiry))
            continue;

        memcpy(szVendor, Inquiry.VendorId, 8);
        memcpy(szProductId, Inquiry.ProductId, 16);
        memcpy(szProductRev, Inquiry.ProductRevisionLevel, 4);

        IoAddHeaderString(0, 0, L"(%s\\) %S %S", szDrive, szVendor, szProductId);

        IoAddValueName(1, 0, IDS_CDROM_FIRMWARE_REV);
        IoSetItemText(L"%S", szProductRev);

        GetSerialNumber(hHandle, szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_CDROM_SERIAL_NUMBER);
            IoSetItemText(szText);
        }

        if (GetInterfaceType(hHandle, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_CDROM_INTERFACE);
            IoSetItemText(szText);
        }

        GetScsiVendorById(szVendor, szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_CDROM_MANUFACTURER);
            IoSetItemText(szText);
        }

        if (GetCDCapabilitiesScsi(hHandle, &Capabilities))
        {
            BYTE tmp = *(BYTE*)&Capabilities.BufferSize;

            *(BYTE*)&Capabilities.BufferSize = *((BYTE*)&Capabilities.BufferSize + 1);
            *((BYTE*)&Capabilities.BufferSize + 1) = tmp;

            IoAddValueName(1, 0, IDS_CDROM_BUFFER_SIZE);
            IoSetItemText(L"%d KB", Capabilities.BufferSize);
        }

        if (GetCDReportKeyScsi(hHandle, &KeyData))
        {
            IoAddValueName(1, 2, IDS_CDROM_REGION_CODE);
            if (KeyData.TypeCode != 0)
                IoSetItemText(L"%d", KeyData.TypeCode);
            else
                IoSetItemText(L"No");

            IoAddValueName(1, 2, IDS_CDROM_REMAINING_USER_CHANGES);
            IoSetItemText(L"%d", KeyData.UserChanges);

            IoAddValueName(1, 2, IDS_CDROM_REMAINING_VENDOR_CHANGES);
            IoSetItemText(L"%d", KeyData.VendorResets);
        }

        IoAddHeader(1, 1, IDS_CDROM_SUP_DISK_TYPES);

        FillReadWriteSupport(hHandle,
                             Capabilities,
                             &rw);

        GetReadWriteFeature(L"BD-ROM",     rw.BD_ROM_Read,     rw.BD_ROM_Write);
        GetReadWriteFeature(L"BD-R",       rw.BD_R_Read,       rw.BD_R_Write);
        GetReadWriteFeature(L"BD-RE",      rw.BD_RE_Read,      rw.BD_RE_Write);
        GetReadWriteFeature(L"HD DVD-ROM", rw.HD_DVD_ROM_Read, rw.HD_DVD_ROM_Write);
        GetReadWriteFeature(L"HD DVD-R",   rw.HD_DVD_R_Read,   rw.HD_DVD_R_Write);
        GetReadWriteFeature(L"HD DVD-RW",  rw.HD_DVD_RW_Read,  rw.HD_DVD_RW_Write);
        GetReadWriteFeature(L"DVD-ROM",    rw.DVD_ROM_Read,    rw.DVD_ROM_Write);
        GetReadWriteFeature(L"DVD+R DL",   rw.DVD_P_R_DL_Read, rw.DVD_P_R_DL_Write);
        GetReadWriteFeature(L"DVD+R",      rw.DVD_P_R_Read,    rw.DVD_P_R_Write);
        GetReadWriteFeature(L"DVD+RW",     rw.DVD_P_RW_Read,   rw.DVD_P_RW_Write);
        GetReadWriteFeature(L"DVD-R DL",   rw.DVD_R_DL_Read,   rw.DVD_R_DL_Write);
        GetReadWriteFeature(L"DVD-R",      rw.DVD_R_Read,      rw.DVD_R_Write);
        GetReadWriteFeature(L"DVD-RW",     rw.DVD_RW_Read,     rw.DVD_RW_Write);
        GetReadWriteFeature(L"DVD-RAM",    rw.DVD_RAM_Read,    rw.DVD_RAM_Write);
        GetReadWriteFeature(L"CD-ROM",     rw.CD_ROM_Read,     rw.CD_ROM_Write);
        GetReadWriteFeature(L"CD-R",       rw.CD_R_Read,       rw.CD_R_Write);
        GetReadWriteFeature(L"CD-RW",      rw.CD_RW_Read,      rw.CD_RW_Write);

        IoAddHeader(1, 0, IDS_CDROM_DRIVE_FEATURES);

        /* SMART */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_SMART);
        IoAddItem(2, 0, L"SMART");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Power Management */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_POWER_MANAGEMENT);
        IoAddItem(2, 0, L"Power Management");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* CD-Text */
        IoAddItem(2, 0, L"CD-Text");
        IsSupported = FALSE;
        if (GetFeatureData(hHandle, FEATURE_CD_READ, &Config))
        {
            if (Config.Feature.additionalData[0] == 3 ||
                Config.Feature.additionalData[0] == 1)
            {
                IsSupported = TRUE;
            }
        }
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* CSS */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_DVD_CSS);
        IoAddItem(2, 0, L"CSS");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* CPRM */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_DVD_CPRM);
        IoAddItem(2, 0, L"CPRM");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* AACS */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_AACS);
        IoAddItem(2, 0, L"AACS");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /*VCPS */
        IsSupported = IsFeatureSupported(hHandle, FEATURE_VCPS);
        IoAddItem(2, 0, L"VCPS");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        CloseScsi(hHandle);
    }

    DebugEndReceiving();
}
