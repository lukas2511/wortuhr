#/usr/bin/python3

segments = {
    'ES': (1, 6),
    'IST': (1, 8),
    'FUENF_1': (1, 7),
    'ZEHN_1': (1, 12),
    'VOR_1': (1, 9),
    'DREI_1': (1, 5),
    'VIER_1': (1, 1),
    'TEL': (1, 11),
    'NACH': (1, 14),
    'VOR_2': (1, 13),
    'HALB': (1, 10),
    'FUENF_2': (1, 15),
    'ZWEI': (1, 0),

    'SIEBEN': (0, 8),
    'VIER_2': (0, 7),
    'ZEHN_2': (0, 9),
    'SECHS': (0, 6),
    'DREI_2': (0, 11),
    'ACHT': (0, 12),
    'ELF': (0, 10),
    'NEUN': (0, 5),
    'EIN': (0, 1),
    'S': (0, 0),
    'ZWOELF': (0, 13),
    'UHR': (0, 14),
}

hours = ['EIN', 'ZWEI', 'DREI', 'VIER_2', 'FUENF_2', 'SECHS', 'SIEBEN', 'ACHT', 'NEUN', 'ZEHN_2', 'ELF', 'ZWOELF']

wortuhr_h = open("wortuhr.h", "w")

segment_pins = "#define SEGMENT_PINS ";

helpers = ""

for segment_name, pincombo in segments.items():
    if pincombo[0] == 1:
        helpers += "#define %s top_half ^= (1 << %d);\n" % (segment_name, pincombo[1])
    else:
        helpers += "#define %s bottom_half |= (1 << %d);\n" % (segment_name, pincombo[1])
    segment_pins += "GPIO%d | " % pincombo[1]

helpers += "\n"
segment_pins = segment_pins[:-3] + "\n"

wortuhr_h.write("uint16_t top_half = 0xFFFF;\n")
wortuhr_h.write("uint16_t bottom_half = 0x0000;\n")
wortuhr_h.write("#define CLEAR_SCREEN top_half = 0xFFFF; bottom_half=0x0000;\n\n");
wortuhr_h.write(helpers)
wortuhr_h.write(segment_pins)

