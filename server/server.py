from flask import Flask
from flask import request, jsonify

from models import AccessPoint, Location, Sample, FingerprintValue, CalibratingMobile
from datetime import datetime
from math import *

app = Flask(__name__)

from database import init_db, db_session
init_db()

@app.teardown_appcontext
def shutdown_session(exception=None):
    db_session.remove()

@app.route("/rssi", methods=['GET', 'POST'])
def rssi():
    # """
	# 	TODO: Implement this function
	# 	It receives data from the access points on the path /rssi
	# 	with a parameter ap whose value is the sending AP MAC address
	# 	and a series of pairs XX:XX:XX:XX:XX:XX=-YY.YYYY
	# 	where the X's are the measured devices MAC addresses
	# 		  and the Y's are the avg RSSI values for the corresponding
	# 		  MAC address over the last second
	# 	You have to put these information in the sqlite3 database
	# 	named rssi.db whose schema can be displayed from the sqlite3
	# 	prompt through the command .schema
	# 	SQL Alchemy ORM classes and initialization are available above
	# """
    if request.method == 'GET':
        data = request.args.to_dict()
        accessPoint = AccessPoint(mac_address=data['ap'])
        for accPoint in data:
            if accPoint != 'ap':
                if not AccessPoint.query.filter_by(mac_address=accessPoint.mac_address).first():
                    try:
                        db_session.add(accessPoint)
                        db_session.commit()
                    except:
                        return "Error1"
                
                accpt = AccessPoint.query.filter_by(mac_address=accessPoint.mac_address).first()
                sample = Sample(ap_id=accpt.id, source_address=accPoint, timestamp=datetime.timestamp(datetime.now()), rssi=float(data[accPoint]), ap=accpt)

                try:
                    db_session.add(sample)
                    db_session.commit()
                except:
                    return "Error2"
    return "OK"


@app.route("/start_calibration", methods=['GET', 'POST'])
def start_calibration():
    """
        TODO: implement this function
        It receives 4 parameters: mac_addr (string), x (float), y (float), and z (float)
        then must trigger 3 tasks:
        (1) Add MAC address and location to table calibrating_mobile
        (2) Find all samples in table sample, and whose source address matches mac_addr
            and whose timestamp is less than 1 second in the past.
            With those samples, insert into table fingerprint_value all entries with
            ap_id matching the AP that forwared the RSSI sample, location the device location
            and RSSI the RSSI from table sample
        (3) In /rssi route: add instructions that process all incoming RSSI samples like
            step (2) when received.
    """
    if request.method == 'GET':
        data = request.args.to_dict()
        accessPoint = AccessPoint(mac_address=data['mac_addr'])
        location = Location(x=data['x'], y=data['y'], z=data['z'])

        if not Location.query.filter_by(x=location.x, y=location.y, z=location.z).first():
            try:
                db_session.add(location)
                db_session.commit()
            except:
                return "Error 1"
        
        if not AccessPoint.query.filter_by(mac_address=accessPoint.mac_address).first():
            try:
                db_session.add(accessPoint)
                db_session.commit()
            except:
                return "Error 2"

        if not CalibratingMobile.query.filter_by(mac_address=accessPoint.mac_address).first():
            try:
                loc = Location.query.filter_by(x=location.x, y=location.y, z=location.z).first()
                calibration = CalibratingMobile(mac_address=accessPoint.mac_address, loc_id=loc.id)
                db_session.add(calibration)
                db_session.commit()
            except:
                return "Error 3"

        sameMaccAddressSamples = []
        allSamples = Sample.query.all()
        now = datetime.timestamp(datetime.now())
        for oneSample in allSamples:
            if(oneSample.source_address == accessPoint.mac_address and oneSample.timestamp - now < 1):
                sameMaccAddressSamples.append(oneSample)

        for oneSample in sameMaccAddressSamples:
            locID = Location.query.filter_by(x=location.x, y=location.y, z=location.z).first().id
            if not FingerprintValue.query.filter_by(loc_id=locID, ap_id=oneSample.ap_id, rssi=oneSample.rssi).first():
                fingerprint = FingerprintValue(loc_id=locID, ap_id=oneSample.ap_id, rssi=oneSample.rssi, location=location, ap=oneSample.ap)
                try:
                    db_session.add(fingerprint)
                    db_session.commit()
                except:
                    return "Error 4"

    return "OK"


@app.route("/stop_calibration", methods=['GET', 'POST'])
def stop_calibration():
    """
        TODO: implement this function
        It receives one parameter: mac_addr (string)
        It must delete any calibrating_mobile entry whose mac_address equal parameter mac_addr
    """
    if request.method == 'GET':
        data = request.args.to_dict()
        macAddress = data['mac_addr']
        allCalibrations = CalibratingMobile.query.all()
        for oneCalibration in allCalibrations:
            if oneCalibration.mac_address == macAddress:
                db_session.delete(oneCalibration)
        db_session.commit()

    return "OK"

def rssi_distance(oneFingerprint, sameMaccAddressSamples, lengthFingerprints):
    """
    Function rssi_distance returns the average RSSI distance between itself and one other sample
    If a sample is missing for an access point in self or other, treat it as -95 dBm
    :param other: the list of AP samples for the other measurement
    :return: the RSSI distance as a floating point value
    """
    common_count = 0
    quad_sum = 0.0
    for oneSample in sameMaccAddressSamples:
        maID = oneSample.ap_id
        if maID != -1:
            common_count += 1
            quad_sum += (oneSample.rssi - oneFingerprint.rssi)**2
    quad_sum += (95**2) * (lengthFingerprints + len(sameMaccAddressSamples) - 2*common_count)
    return sqrt(quad_sum)

@app.route("/locate", methods=['GET', 'POST'])
def locate():
    """
        TODO: implement this function
        It receives one parameter: mac_addr (string)
        Must locate the device based on samples less than 1 second old, whose source address equals mac_addr
        These samples are compared to the content of fingerprint_value table
        Use the closest in RSSI algorithm to find a fingerprint sample matching current sample and return its location
    """
    if request.method == 'GET':
        data = request.args.to_dict()
        macAddress = data['mac_addr']

        sameMaccAddressSamples = []
        allSamples = Sample.query.all()
        now = datetime.timestamp(datetime.now())
        for oneSample in allSamples:
            if(oneSample.source_address == macAddress and oneSample.timestamp - now < 1):
                sameMaccAddressSamples.append(oneSample)

        allFingerprints = FingerprintValue.query.all()

        minDist = 2000
        location = Location()

        for oneFingerprint in allFingerprints:
            distance = rssi_distance(oneFingerprint, sameMaccAddressSamples, len(allFingerprints))
            if distance < minDist:
                minDist = distance
                location = oneFingerprint.location

    result = "Location is :  x:{} y:{} z:{}".format(location.x,location.y,location.z)
    return result

