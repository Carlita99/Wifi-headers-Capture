from sqlalchemy import create_engine, Column, Integer, String, Float, ForeignKey
from sqlalchemy.orm import relationship, sessionmaker, scoped_session
from database import Base

class AccessPoint(Base):
    __tablename__ = "accesspoint"
    id = Column(Integer, primary_key=True)
    mac_address = Column(String)

class Location(Base):
    __tablename__ = "location"
    id = Column(Integer, primary_key=True)
    x = Column(Float)
    y = Column(Float)
    z = Column(Float)

class Sample(Base):
    __tablename__ = "sample"
    ap_id = Column(Integer, ForeignKey("accesspoint.id"))
    source_address = Column(String, nullable=False, primary_key=True)
    timestamp = Column(Float, nullable=False, primary_key=True)
    rssi = Column(Float, nullable=False)
    ap = relationship("AccessPoint", backref="sample")

    def values(self, src, t, _rssi, _ap):
        source_address = src
        timestamp = t
        rssi = _rssi
        ap = _ap

class FingerprintValue(Base):
    __tablename__ = "fingerprint_value"
    id = Column(Integer, primary_key=True)
    loc_id = Column(Integer, ForeignKey("location.id"))
    ap_id = Column(Integer, ForeignKey("accesspoint.id"))
    rssi = Column(Float, nullable=False)
    location = relationship("Location", backref="fingerprint_value")
    ap = relationship("AccessPoint", backref="fingerprint_value")

class CalibratingMobile(Base):
    __tablename__ = "calibrating_mobile"
    mac_address = Column(String, primary_key=True)
    loc_id = Column(Integer, ForeignKey("location.id"))
    location = relationship("Location", backref="calibrating_mobile")