fs = require 'fs'

data = []

lines = fs.readFileSync('./tempResult', {encoding: 'utf-8'}).split "\n"
for i in [0..lines.length] by 2
  [nodes, tpn, rounds] = lines[i].split ' - '
  tpn = tpn - 1
  nodes = nodes - 1
  time = parseFloat lines[i + 1]
  if (tpn and nodes and rounds)
    if not data[tpn]
      data[tpn] = []
    if not data[tpn][nodes]
      data[tpn][nodes] = []
    data[tpn][nodes][rounds] = time

displayTable = (tpn) ->
  for node in data[tpn]
    console.log node.length

displayTable 1
