import math
import json
import random
import argparse

def genRandomFeatures(n):
  features = []
  for i in range(0, n):
    lat = (random.random() - 0.5) * 360.0
    lng = (random.random() - 0.5) * 180.0
    geom = { 'type': 'Point', 'coordinates': [lat, lng] }
    props = { 'class': 1 if random.random() > 0.5 else 0 }
    feature = { 'type': 'Feature', 'properties': props, 'geometry': geom }
    features.append(feature)
  return features

def genGridFeatures(nx, ny):
  features = []
  for i in range(0, nx):
    for j in range(0, ny):
      lat = (i - 0.5) * 360.0 / nx
      lng = (j - 0.5) * 180.0 / ny
      geom = { 'type': 'Point', 'coordinates': [lat, lng] }
      props = { 'class': 1 if random.random() > 0.5 else 0 }
      feature = { 'type': 'Feature', 'properties': props, 'geometry': geom }
      features.append(feature)
  return features

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument(dest='tableName', help='The name of the db table')
  parser.add_argument(dest='numPoints', type=int, help='The number of random points')

  args = parser.parse_args()

  features = genRandomFeatures(args.numPoints)

  print("DROP TABLE IF EXISTS %s;" % args.tableName)
  print("CREATE TABLE %s(gid serial PRIMARY KEY, geom GEOMETRY, attr NUMERIC);" % args.tableName)
  for feature in features:
    geom = "POINT(%g %g)" % tuple(feature['geometry']['coordinates'])
    print("INSERT INTO %s VALUES (DEFAULT, GeomFromEWKT('SRID=4326;%s'), %d);" % (args.tableName, geom, feature['properties']['class']))

if __name__ == "__main__":
  main()
