from typing import *
import json
import sys

def format_val(code: TextIO, format: dict[str, Any], defs: dict[str, Any]) -> None:
    if 'id' in format:
        format = defs[format['id']]

    type: str = format['type']
    if type == 'divide':
        value: int = int(format['value'])
        code.write('val = val.toInt() / {}.0f;\n'.format(value))
    elif type == 'enum':
        options: dict[str, str] = format['options']
        code.write('switch (val.toUInt()) {\n')

        val: str
        label: str
        for val, label in options.items():
            if val.isdigit():
                code.write('case {}U:\n'.format(val))
            else:
                code.write('case "{}"_ts.value:\n'.format(val))
            code.write('val = u"{}"_s;\n'.format(label))
            code.write('break;\n')
        code.write('}\n')
    elif type == 'flags':
        code.write('// TODO: flags\n')
    elif type == 'AtxtPositionFormat':
        code.write('// TODO\n')
    elif type == 'ClmtMoonsPhaseLengthFormat':
        code.write('// TODO\n')
    elif type == 'ClmtTimeFormat':
        code.write('// TODO\n')
    elif type == 'CloudSpeedFormat':
        code.write('// TODO\n')
    elif type == 'CTDAFunctionFormat':
        code.write('// TODO\n')
    elif type == 'CtdaTypeFormat':
        code.write('// TODO\n')
    elif type == 'Edge0Format':
        code.write('// TODO\n')
    elif type == 'Edge1Format':
        code.write('// TODO\n')
    elif type == 'Edge2Format':
        code.write('// TODO\n')
    elif type == 'HideFFFF_Format':
        code.write('// TODO\n')
    elif type == 'NextObjectIDFormat':
        code.write('// TODO\n')
    elif type == 'QuestAliasFormat':
        code.write('// TODO\n')
    elif type == 'QuestExternalAliasFormat':
        code.write('// TODO\n')
    elif type == 'REFRNavmeshTriangleFormat':
        code.write('// TODO\n')
    elif type == 'ScriptObjectAliasFormat':
        code.write('// TODO\n')
    elif type == 'TintLayerFormat':
        code.write('// TODO: TintLayerFormat\n')
    elif type == 'Vertex0Format':
        code.write('// TODO\n')
    elif type == 'Vertex1Format':
        code.write('// TODO\n')
    elif type == 'Vertex2Format':
        code.write('// TODO\n')
    else:
        code.write('#pragma message("warning: unhandled format type {}")\n'.format(type))

def define_type(code: TextIO, element: dict[str, Any], defs: dict[str, Any]) -> None:
    if 'id' in element:
        element = defs[element['id']]

    type: str = element['type']
    if type == 'int0':
        code.write('QVariant val = 0;\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'int8':
        code.write('QVariant val = '
                   'TESFile::readType<std::int8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'int16':
        code.write('QVariant val = TESFile::readType<std::int16_t>(*stream);\n')
        if 'name' in element and element['name'] == 'Object Format':
            code.write('ObjectFormat = val.toInt();\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'int32':
        code.write('QVariant val = TESFile::readType<std::int32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'uint8':
        code.write('QVariant val = TESFile::readType<std::uint8_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'uint16':
        code.write('QVariant val = TESFile::readType<std::uint16_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'uint32':
        code.write('QVariant val = TESFile::readType<std::uint32_t>(*stream);\n')
        if 'format' in element:
            format: dict[str, Any] = element['format']
            format_val(code, format, defs)
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'float':
        code.write('QVariant val = TESFile::readType<float>(*stream);\n')
        code.write('item->setDisplayData(fileIndex, val);\n')

    elif type == 'string':
        localized: bool = 'localized' in element and element['localized']
        if localized:
            code.write('item->setDisplayData('
                       'fileIndex, readLstring(localized, *stream));\n')
        elif 'prefix' in element:
            prefix: int = element['prefix']
            lengthType: str
            if prefix == 1:
                lengthType = 'std::uint8_t'
            elif prefix == 2:
                lengthType = 'std::uint16_t'
            elif prefix == 4:
                lengthType = 'std::uint32_t'
            code.write((
                'const auto length = TESFile::readType<{}>(*stream);\n'
                'std::string str;\n'
                'str.resize(length);\n'
                'stream->read(str.data(), length);\n'
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n'
                ).format(lengthType))
        else:
            code.write(
                'std::string str;\n'
                "std::getline(*stream, str, '\\0');\n"
                'item->setDisplayData(fileIndex, QString::fromStdString(str));\n')
    elif type == 'formId':
        code.write('item->setDisplayData('
                   'fileIndex, readFormId(masters, plugin, *stream));\n')
    elif type == 'bytes':
        size: int = element['size'] if 'size' in element else 256
        code.write('item->setDisplayData(fileIndex, readBytes(*stream, {}));\n'.format(
            size))
    elif type == 'array':
        name: str = element['name']
        code.write('if (stream->peek() != std::char_traits<char>::eof()) {\n')
        if 'count' in element:
            count: int = element['count']
            code.write(('for ([[maybe_unused]] int i_{} : '
                        'std::ranges::iota_view(0, {})) {{\n'
                        ).format(name.replace(' ', ''), count))
        elif 'counter' in element:
            nameId: str = name.replace(' ', '')
            counter: dict[str, Any] = element['counter']
            counterType: str = counter['type']
            if counterType == 'elementCounter':
                path: str = counter['path']
                code.write(('const int count_{} = '
                            'root->childData(u"{}"_s, fileIndex).toInt();\n'
                            ).format(nameId, path))
            elif counterType == 'ScriptFragmentsInfoCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n'
                            ).format(nameId))
            elif counterType == 'ScriptFragmentsPackCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x7U);\n'
                            ).format(nameId))
            elif counterType == 'ScriptFragmentsQuestCounter':
                code.write(('const int count_{} = '
                            'item->parent()->childData('
                            'u"FragmentCount"_s, fileIndex)'
                            '.toInt();\n').format(nameId))
            elif counterType == 'ScriptFragmentsSceneCounter':
                code.write(('const int count_{} = std::popcount('
                            'item->parent()->childData('
                            'u"Flags"_s, fileIndex).toUInt() & 0x3U);\n'
                            ).format(nameId))
            else:
                code.write('#pragma message("warning: unknown counter type {}")'.format(
                    counterType))
            code.write(('for ([[maybe_unused]] int i_{0} : '
                        'std::ranges::iota_view(0, count_{0})) {{\n'
                        ).format(nameId))
        elif 'prefix' in element:
            nameId: str = name.replace(' ', '').replace('?', '')
            prefix: int = element['prefix']
            if prefix == 1:
                code.write(('const int count_{} = '
                           'TESFile::readType<std::uint8_t>(*stream);\n'
                           ).format(nameId))
            elif prefix == 2:
                code.write(('const int count_{} = '
                            'TESFile::readType<std::uint16_t>(*stream);\n'
                            ).format(nameId))
            elif prefix == 4:
                code.write(('const int count_{} = '
                            'TESFile::readType<std::uint32_t>(*stream);\n'
                            ).format(nameId))
            code.write(('for ([[maybe_unused]] int i_{0} : '
                        'std::ranges::iota_view(0, count_{0})) {{\n'
                        ).format(nameId))
        else:
            code.write('while (!stream->eof()) {\n')
        arrayElement: dict[str, Any] = element['element']
        if 'id' in arrayElement:
            arrayElement = defs[arrayElement['id']]
        code.write(('item = item->getOrInsertChild('
                    'indexStack.back()++, u"{}"_s);\n').format(arrayElement['name']))
        code.write('indexStack.push_back(0);\n')
        define_type(code, arrayElement, defs)
        code.write('item = item->parent();\n')
        code.write('indexStack.pop_back();\n\n')
        code.write('}\n')
        code.write('}\n')
    elif type == 'struct':
        name: str = element['name']

        code.write('if (stream->peek() != std::char_traits<char>::eof()) {\n')
        structElements: list[Any] = element['elements']
        structElement: dict[str, Any]
        for structElement in structElements:
            if 'id' in structElement:
                structElement = {**structElement, **defs[structElement['id']]}
                del structElement['id']
            conflictType: str = (structElement['conflictType']
                                 if 'conflictType' in structElement
                                 else 'Override')
            elementName: str = structElement['name']
            code.write(('item = item->getOrInsertChild('
                        'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                        ).format(elementName, conflictType))
            code.write('indexStack.push_back(0);\n')
            code.write('{\n')
            define_type(code, structElement, defs)
            code.write('}\n')
            code.write('item = item->parent();\n')
            code.write('indexStack.pop_back();\n\n')
        code.write('}\n')
    elif type == 'union':
        decider: str = element['decider']
        code.write('[[maybe_unused]] int decider = -1;\n')
        if decider == 'GMSTUnionDecider':
            code.write('enum {Name, Int, Float, Bool};\n'
                       'switch (root->childData("EDID"_ts, fileIndex)'
                       '.toString()[0].unicode()) {\n'
                       "case u'b': decider = Bool; break;\n"
                       "case u'f': decider = Float; break;\n"
                       "case u'i': decider = Int; break;\n"
                       "case u'u': decider = Int; break;\n"
                       "case u's': decider = Name; break;\n"
                       '}\n')
        elif decider == 'CTDACompValueDecider':
            code.write('// TODO: CTDACompValueDecider\n')
            return
        elif decider == 'CTDAParam1Decider':
            code.write('// TODO: CTDAParam1Decider\n')
            return
        elif decider == 'CTDAParam2Decider':
            code.write('// TODO: CTDAParam1Decider\n')
            return
        elif decider == 'CTDAReferenceDecider':
            code.write('// TODO: CTDAReferenceDecider\n')
            return
        elif decider == 'ScriptPropertyDecider':
            code.write('enum {Unused, ObjectUnion, String, Int32, Float, Bool, '
                       'ArrayofObject, ArrayofString, ArrayofInt32, ArrayofFloat, '
                       'ArrayofBool};\n'
                       'const QString propertyType = '
                       'item->parent()->childData(u"Type"_s, fileIndex).toString();\n'
                       'if (propertyType == u"None"_s) {\n'
                       'decider = Unused;\n'
                       '} else if (propertyType == u"Object"_s) {\n'
                       'decider = ObjectUnion;\n'
                       '} else if (propertyType == u"String"_s) {\n'
                       'decider = String;\n'
                       '} else if (propertyType == u"Int32"_s) {\n'
                       'decider = Int32;\n'
                       '} else if (propertyType == u"Float"_s) {\n'
                       'decider = Float;\n'
                       '} else if (propertyType == u"Bool"_s) {\n'
                       'decider = Bool;\n'
                       '} else if (propertyType == u"Array of Object"_s) {\n'
                       'decider = ArrayofObject;\n'
                       '} else if (propertyType == u"Array of String"_s) {\n'
                       'decider = ArrayofString;\n'
                       '} else if (propertyType == u"Array of Int32"_s) {\n'
                       'decider = ArrayofInt32;\n'
                       '} else if (propertyType == u"Array of Float"_s) {\n'
                       'decider = ArrayofFloat;\n'
                       '} else if (propertyType == u"Array of Bool"_s) {\n'
                       'decider = ArrayofBool;\n'
                       '}\n')
        elif decider == 'ScriptObjFormatDecider':
            code.write('enum {Objectv2, Objectv1};\n'
                       'decider = ObjectFormat == 1 ? Objectv1 : Objectv2;')
        elif decider == 'TypeDecider':
            code.write('// TODO: TypeDecider\n')
            return
        elif decider == 'BOOKTeachesDecider':
            code.write('// TODO: BOOKTeachesDecider\n')
            return
        elif decider == 'COEDOwnerDecider':
            code.write('// TODO: COEDOwnerDecider\n')
            return
        elif decider == 'MGEFAssocItemDecider':
            code.write('// TODO: MGEFAssocItemDecider\n')
            return
        elif decider == 'NAVIIslandDataDecider':
            code.write('// TODO: NAVIIslandDataDecider\n')
            return
        elif decider == 'NAVIParentDecider':
            code.write('// TODO: NAVIParentDecider\n')
            return
        elif decider == 'NVNMParentDecider':
            code.write('// TODO: NVNMParentDecider\n')
            return
        elif decider == 'NPCLevelDecider':
            code.write('// TODO: NPCLevelDecider\n')
            return
        elif decider == 'PubPackCNAMDecider':
            code.write('// TODO: PubPackCNAMDecider\n')
            return
        elif decider == 'PerkDATADecider':
            code.write('// TODO: PerkDataDecider\n')
            return
        elif decider == 'EPFDDecider':
            code.write('// TODO: EPFDDecider\n')
            return
        else:
            code.write('// TODO: union decider {}\n'.format(decider))
            return

        unionElements: list[Any] = element['elements']
        unionElement: dict[str, Any]
        code.write('switch (decider) {\n')
        for unionElement in unionElements:
            if 'id' in unionElement:
                unionElement = defs[unionElement['id']]
            name: str = unionElement['name'].replace(' ', '')
            code.write('case {}: {{\n'.format(name))
            define_type(code, unionElement, defs)
            code.write('} break;\n')
        code.write('}\n')
    elif type == 'empty':
        code.write('// TODO: empty\n')
    elif type == 'memberStruct':
        #define_member(code, element, defs)
        members: list[Any] = element['members']
        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                define_member(code, structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                code.write(('item = item->getOrInsertChild('
                            'indexStack.back()++, "{}"_ts, u"{}"_s);\n'
                            ).format(structSig, structName))
                code.write('indexStack.push_back(0);\n')
                code.write('if (signature == "{}"_ts) {{\n'.format(structSig))
                define_type(code, structMember, defs)
                code.write('co_await std::suspend_always();\n}\n')
                code.write('item = item->parent();\n')
                code.write('indexStack.pop_back();\n\n')
    elif type == 'memberUnion':
        define_member(code, element, defs)
    else:
        code.write('#pragma message("warning: unhandled type {}")\n'.format(type))

def member_condition(member: dict[str, Any], defs: dict[str, Any]) -> str:
    if 'id' in member:
        member = defs[member['id']]
    memberType = member['type']
    if memberType == 'memberArray':
        return member_condition(member['member'], defs)
    if memberType == 'memberStruct':
        return member_condition(member['members'][0], defs)
    elif memberType == 'memberUnion':
        return ' || '.join(
            (member_condition(unionMember, defs)
             for unionMember in member['members']))
    else:
        signature: str = member['signature'].encode('unicode_escape').decode()
        return 'signature == "{}"_ts'.format(signature)

def define_member(code: TextIO, member: dict[str, Any], defs: dict[str, Any]) -> None:
    name: str = member['name'] if 'name' in member else 'Unknown'
    conflictType: str = (member['conflictType'] if 'conflictType' in member
                         else 'Override')

    alignable: bool = True
    if 'defFlags' in member:
        defFlags: list[str] = member['defFlags']
        if 'notAlignable' in defFlags:
            alignable = False

    type: str = member['type']
    if type == 'memberArray':
        arrayMember: dict[str, Any] = member['member']
        if 'id' in arrayMember:
            arrayMember = defs[arrayMember['id']]
        arrayName: str = ''
        if 'name' in arrayMember:
            arrayName = arrayMember['name']
        code.write(('item = item->getOrInsertChild('
                    'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                    ).format(name, conflictType))
        code.write('indexStack.push_back(0);\n')
        code.write('while ({}) {{\n'.format(member_condition(arrayMember, defs)))

        arrayType: str = arrayMember['type']
        if arrayType.startswith('member'):
            define_member(code, arrayMember, defs)
        else:
            func: str = 'getOrInsertChild' if alignable else 'insertChild'
            if 'signature' in arrayMember:
                arraySig: str = arrayMember['signature'].encode('unicode_escape').decode()
                code.write(('item = item->{}('
                            'indexStack.back()++, "{}"_ts, u"{}"_s, '
                            'ConflictType::{});\n'
                            ).format(func, arraySig, arrayName, conflictType))
            else:
                code.write(('item = item->{}('
                            'indexStack.back()++, u"{}"_s);\n').format(func, arrayName))
            code.write('indexStack.push_back(0);\n')

            define_type(code, arrayMember, defs)
            code.write('item = item->parent();\n')
            code.write('indexStack.pop_back();\n\n')

            if 'signature' in arrayMember:
                code.write('co_await std::suspend_always();\n')
        code.write('}\n')
        code.write('item = item->parent();\n')
        code.write('indexStack.pop_back();\n\n')
    elif type == 'memberStruct':
        code.write(('item = item->getOrInsertChild('
                    'indexStack.back()++, u"{}"_s, ConflictType::{});\n'
                    ).format(name, conflictType))
        code.write('indexStack.push_back(0);\n')
        members: list[Any] = member['members']

        structMember: dict[str, Any]
        for structMember in members:
            if 'id' in structMember:
                structMember = {**structMember, **defs[structMember['id']]}
                del structMember['id']
            structType: str = structMember['type']
            if structType.startswith('member'):
                define_member(code, structMember, defs)
            else:
                structSig: str = structMember['signature'].encode('unicode_escape').decode()
                structName: str = ''
                if 'name' in structMember:
                    structName: str = structMember['name']
                code.write(('item = item->getOrInsertChild('
                            'indexStack.back()++, "{}"_ts, u"{}"_s);\n'
                            ).format(structSig, structName))
                code.write('indexStack.push_back(0);\n')
                code.write('if (signature == "{}"_ts) {{\n'.format(structSig))
                define_type(code, structMember, defs)
                code.write('co_await std::suspend_always();\n}\n')
                code.write('item = item->parent();\n')
                code.write('indexStack.pop_back();\n\n')
        code.write('item = item->parent();\n')
        code.write('indexStack.pop_back();\n\n')
    elif type == 'memberUnion':
        code.write(('item = item->getOrInsertChild('
                    'indexStack.back()++, u"{}"_s);\n').format(name))
        code.write('indexStack.push_back(0);\n')
        members: list[Any] = member['members']

        unionMember: dict[str, Any]
        for unionMember in members:
            if 'id' in unionMember:
                unionMember = defs[unionMember['id']]
            unionType: str = unionMember['type']
            code.write('if ({}) {{\n'.format(member_condition(unionMember, defs)))
            if unionType.startswith('member'):
                define_member(code, unionMember, defs)
            else:
                unionSig: str = unionMember['signature'].encode('unicode_escape').decode()
                unionName: str = ''
                if 'name' in unionMember:
                    unionName: str = unionMember['name']
                code.write('if (signature == "{}"_ts) {{\n'.format(unionSig))
                code.write(('item = item->getOrInsertChild('
                            'indexStack.back()++, "{}"_ts, u"{}"_s);\n'
                            ).format(unionSig, unionName))
                code.write('indexStack.push_back(0);\n')
                define_type(code, unionMember, defs)
                code.write('item = item->parent();\n')
                code.write('indexStack.pop_back();\n\n')
                code.write('co_await std::suspend_always();\n}\n')
            code.write('}\n')
        code.write('item = item->parent();\n')
        code.write('indexStack.pop_back();\n\n')
    else:
        signature: str = member['signature'].encode('unicode_escape').decode()
        if signature == 'VMAD':
            code.write('int ObjectFormat;\n')
        code.write(('item = item->getOrInsertChild('
                    'indexStack.back()++, "{}"_ts, u"{}"_s);\n'
                    ).format(signature, name))
        code.write('indexStack.push_back(0);\n')
        code.write('if (signature == "{}"_ts) {{\n'.format(signature))
        define_type(code, member, defs)
        code.write('co_await std::suspend_always();\n}\n')
        code.write('item = item->parent();\n')
        code.write('indexStack.pop_back();\n\n')

def define_record(code: TextIO, definition: dict[str, Any], defs: dict[str, Any]) -> str:
    signature: str = definition['signature'].encode('unicode_escape').decode()
    if 'id' in definition:
        definition = {**definition, **defs[definition['id']]}

    code.write((
        'template <>\n'
        'inline ParseTask FormParser<"{}">::parseForm('
        '    DataItem* root, int fileIndex, [[maybe_unused]] bool localized,\n'
        '    [[maybe_unused]] std::span<const std::string> masters,\n'
        '    [[maybe_unused]] const std::string& plugin, const TESFile::Type& signature,\n'
        '    std::istream* const& stream) const\n'
        '{{\n'
        'using ConflictType = DataItem::ConflictType;'
        'DataItem* item = root;\n'
        'std::vector<int> indexStack{{0}};\n'
        '\n').format(signature))
    members: list[Any] = definition['members']
    member: dict[str, Any]
    for member in members:
        if 'id' in member:
            member = {**member, **defs[member['id']]}
            del member['id']
        define_member(code, member, defs)

    code.write('for (;;) {\n'
               'parseUnknown('
               'root, indexStack.front(), fileIndex, signature, *stream);\n'
               'co_await std::suspend_always();\n'
               '}\n')
    code.write('}\n\n')

if __name__ == '__main__':
    dataPath: str = sys.argv[1]
    outPath: str = sys.argv[2]

    dataFile: TextIO
    with open(dataPath, 'r') as dataFile:
        data: dict[str, Any] = json.load(dataFile)
        defs: dict[str, Any] = data['defs']

        with open(outPath, 'w') as outFile:
            outFile.write('namespace TESData\n{\n\n')

            id: str
            definition: dict[str, Any]
            for id, definition in defs.items():
                type: str = definition['type']
                if type == 'record' and 'signature' in definition:
                    define_record(outFile, definition, defs)

            outFile.write('\n}\n')
