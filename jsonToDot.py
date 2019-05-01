# Converts the JSON AST to Dot file

import json
import argparse
import io

id = 0

def write_name(json_doc, f):
  global id
  if type(json_doc) == str:
    # print(json_doc)
    return
  if type(json_doc) == list:
    for doc in json_doc:
      write_name(doc, f)
  if "name" in json_doc:
    f.write('  "' + str(id) + '" [label=' + str(json_doc["name"]) + ']\n')
    id += 1
  if "data" in json_doc:
    for doc in json_doc["data"]:
      print(doc)
      if doc == "data":
        print(json_doc["data"]["data"])
        write_name(json_doc["data"][doc], f)
      else:
        write_name(doc, f)

def main():
  parser = argparse.ArgumentParser(description='Converts json to dot file.')
  parser.add_argument('file', metavar='F', type=str, nargs=1,
                      help='File to convert')
  args = parser.parse_args()
  file = args.file[0]
  json_doc = None
  with open(file, 'r') as f:
    lines = f.read()
    json_doc = json.loads(lines)
  id = 0
  with open(file + ".dot", 'w') as f:
    f.write("digraph AST {\n")
    write_name(json_doc, f)

    f.write("}\n")

if __name__ == "__main__":
    main()
