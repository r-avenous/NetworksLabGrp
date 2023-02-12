-- Create Tables

create table Physician
(
	EmployeeID INT PRIMARY KEY,
	Name VARCHAR (20) NOT NULL,
	Position VARCHAR (20) NOT NULL,
	SSN INT NOT NULL
);
create table Department
(
	DepartmentID int primary key,
	Name varchar (20) not null,
	Head int,
	foreign key (Head) references physician (EmployeeID)
);
create table Procedure
(
	Code int primary key,
	Name varchar (20) not null,
	Cost int not null
);
create table Affiliated_with
(
	Physician int,
	Department int,
	PrimaryAffiliation bool not null,
	primary key (Physician, Department),
	foreign key (Physician) references physician (EmployeeID), 
	foreign key (Department) references department (DepartmentID)
);
create table Patient
(
	SSN int primary key,
	Name varchar (20) not null,
	Address varchar (50) not null,
	Phone char (10) not null,
	InsuranceID int not null,
	PCP int references Physician(EmployeeID)
);
create table Block
(
	Floor int,
	Code int,
	primary key (Floor, Code)
);
create table Nurse
(
	EmployeeID int primary key,
	Name varchar (20) not null,
	Position varchar (20) not null,
	Registered bool not null,
	SSN int not null
);
create table Medication
(
	Code int primary key,
	Name varchar (20) not null,
	Brand varchar (20) not null,
	Description varchar (50)
);
create table Room
(
	Number int primary key,
	Type varchar (20) not null,
	BlockFloor int,
	BlockCode int,
	Unavailable bool not null,
	foreign key (BlockFloor, BlockCode) references block (Floor, Code)
);
CREATE TABLE Stay
(
	StayID INT PRIMARY KEY,
	Patient INT REFERENCES Patient(SSN),
	Room INT REFERENCES Room(Number),
	Start TIMESTAMP not null,
	"End" TIMESTAMP NOT NULL
);
CREATE TABLE Appointment
(
	AppointmentID INT PRIMARY KEY,
	Patient INT NOT NULL REFERENCES Patient(SSN),
	PrepNurse integer REFERENCES Nurse(EmployeeID),
	Physician INT NOT NULL REFERENCES Physician(EmployeeID),
	Start TIMESTAMP NOT NULL,
	"End" TIMESTAMP NOT NULL,
	ExaminationRoom varchar (20) NOT NULL
);
CREATE TABLE Prescribes
(
	Physician INT NOT NULL REFERENCES Physician(EmployeeID),
	Patient INT NOT NULL REFERENCES Patient(SSN),
	Medication INT NOT NULL REFERENCES Medication(Code),
	Date TIMESTAMP NOT NULL,
	Appointment integer REFERENCES Appointment(AppointmentID),
	Dose varchar (30) NOT NULL,
	PRIMARY KEY(Physician, Patient, Medication, Date)
);
CREATE TABLE On_Call
(
	Nurse INT NOT NULL REFERENCES Nurse(EmployeeID),
	BlockFloor INT NOT NULL,
	BlockCode INT NOT NULL,
	Start TIMESTAMP NOT NULL,
	"End" TIMESTAMP NOT NULL,
	PRIMARY KEY(Nurse, BlockFloor, BlockCode, Start, "End"),
	Foreign Key(BlockFloor, BlockCode) REFERENCES Block(Floor, Code)
);
CREATE TABLE Trained_In
(
	Physician INT NOT NULL REFERENCES Physician(EmployeeID),
	Treatment INT NOT NULL REFERENCES Procedure(Code),
	CertifiactionDate TIMESTAMP NOT NULL,
	CertificationExpires TIMESTAMP NOT NULL,
	PRIMARY KEY(Physician, Treatment)
);
CREATE TABLE Undergoes
(
	Patient INT NOT NULL REFERENCES Patient(SSN),
	Procedure INT NOT NULL REFERENCES Procedure(Code),
	Stay INT NOT NULL REFERENCES Stay(StayID),
	Date TIMESTAMP NOT NULL,
	Physician INT NOT NULL REFERENCES Physician(EmployeeID),
	AssistingNurse integer REFERENCES Nurse(EmployeeID),
	PRIMARY KEY(Patient, Procedure, Stay, Date)
);

-- Populate

insert into physician values (1, 'Walter White', 'druglord', 0);
insert into physician values (2, 'Eddie Brock', 'operator', 69);
insert into physician values (3, 'Darth Vader', 'manager', -2);
insert into physician values (4, 'Tomy Stark', 'director', 155);
insert into physician values (5, 'Peter Parker', 'intern', 89);
insert into physician values (6, 'Max Payne', 'anesthesist', 42);
insert into physician values (7, 'Homelander', 'maniac', 1023);
insert into physician values (8, 'Daya', 'door fixer', 10);

insert into department values (1, 'Cardiology', 4);
insert into department values (2, 'Meth', 1);
insert into department values (3, 'Psychology', 7);

insert into affiliated_with values (1, 1, false);
insert into affiliated_with values (1, 2, true);
insert into affiliated_with values (2, 2, true);
insert into affiliated_with values (3, 1, false);
insert into affiliated_with values (3, 2, true);
insert into affiliated_with values (4, 1, true);
insert into affiliated_with values (5, 3, false);
insert into affiliated_with values (5, 2, false);
insert into affiliated_with values (5, 1, true);
insert into affiliated_with values (6, 1, true);
insert into affiliated_with values (7, 1, false);
insert into affiliated_with values (7, 3, true);
insert into affiliated_with values (8, 3, true);

insert into patient values (32, 'pat1', 'Address1', '1111111111', 23, 6);
insert into patient values (64, 'pat2', 'Address2', '2222222222', 46, 7);
insert into patient values (59, 'pat3', 'Address3', '3333333333', 95, 5);
insert into patient values (11, 'pat4', 'Address4', '4444444444', 11, 5);
insert into patient values (102, 'pat5', 'Address5', '5555555555', 2, 3);

insert into nurse values (9, 'nur9', 'nurse', true, 1);
insert into nurse values (10, 'nur10', 'headnurse', false, 1000);
insert into nurse values (11, 'nur11', 'stillnurse', true, 400);
insert into nurse values (12, 'nur12', 'oldnurse', true, 3000);
insert into nurse values (13, 'nur13', 'comicnurse', false, 2014);
insert into nurse values (14, 'nur14', 'malenurse', true, 2);
insert into nurse values (15, 'nur15', 'doesntnurse', true, 803);

insert into procedure values (1, 'General', 200);
insert into procedure values (2, 'Bypass Surgery', 100000);
insert into procedure values (3, 'Vaccination', 1000);
insert into procedure values (4, 'Cast', 6000);
insert into procedure values (5, 'Physio', 100);

insert into block values (1, 1);
insert into block values (1, 2);
insert into block values (2, 1);

insert into room values (110, 'icu', 1, 1, false);
insert into room values (111, 'icu', 1, 1, false);
insert into room values (112, 'icu', 1, 1, false);
insert into room values (113, 'icu', 1, 1, false);
insert into room values (121, 'general', 1, 2, false);
insert into room values (122, 'general', 1, 2, false);
insert into room values (123, 'general', 1, 2, false);
insert into room values (124, 'general', 1, 2, false);
insert into room values (210, 'deluxe', 2, 1, true);
insert into room values (211, 'deluxe', 2, 1, true);

insert into stay values (1, 102, 112, '2023-01-19 00:00:00', '2023-01-25 00:00:00');
insert into stay values (2, 32, 121, '2022-01-09 00:00:00', '2023-01-09 00:00:00');
insert into stay values (3, 64, 122, '2022-04-22 00:00:00', '2022-07-11 00:00:00');
insert into stay values (4, 32, 121, '2023-01-11 00:00:00', '2023-01-13 00:00:00');
insert into stay values (5, 11, 210, '2022-04-18 00:00:00', '2022-05-19 00:00:00');
insert into stay values (6, 32, 123, '2023-01-15 00:00:00', '2023-01-16 00:00:00');
insert into stay values (7, 11, 211, '2023-07-22 00:00:00', '2022-09-28 00:00:00');
insert into stay values (8, 32, 121, '2023-01-19 00:00:00', '2023-01-25 00:00:00');
insert into stay values (9, 59, 110, '2022-08-01 00:00:00', '2022-11-09 00:00:00');
insert into stay values (10, 59, 110, '2022-12-12 00:00:00', '2023-01-14 00:00:00');

insert into on_call values (9, 1, 2, '1970-01-01', '2070-01-01');
insert into on_call values (10, 2, 1, '1970-01-01', '2070-01-01');
insert into on_call values (11, 2, 1, '1970-01-01', '2070-01-01');
insert into on_call values (12, 1, 1, '1970-01-01', '2070-01-01');
insert into on_call values (13, 1, 2, '1970-01-01', '2070-01-01');
insert into on_call values (14, 1, 1, '1970-01-01', '2070-01-01');
insert into on_call values (15, 1, 1, '1970-01-01', '1980-12-31');
insert into on_call values (15, 1, 2, '1981-01-01', '1991-12-31');
insert into on_call values (15, 2, 1, '1992-01-01', '2002-12-31');
insert into on_call values (15, 1, 1, '2003-01-01', '2013-12-31');
insert into on_call values (15, 1, 2, '2014-01-01', '2024-12-31');

insert into trained_in values (1, 1, '1970-01-01', '2070-01-01');
insert into trained_in values (2, 1, '1970-01-01', '2070-01-01');
insert into trained_in values (3, 1, '1970-01-01', '2070-01-01');
insert into trained_in values (4, 1, '1970-01-01', '2070-01-01');
insert into trained_in values (1, 3, '1970-01-01', '2025-01-01');
insert into trained_in values (1, 4, '1970-01-01', '2025-01-01');
insert into trained_in values (2, 3, '1970-01-01', '2024-01-01');
insert into trained_in values (2, 4, '1970-01-01', '2024-01-01');
insert into trained_in values (3, 3, '1970-01-01', '2024-01-01');
insert into trained_in values (3, 4, '1970-01-01', '2025-01-01');
insert into trained_in values (5, 5, '2000-01-01', '2024-01-01');
insert into trained_in values (7, 5, '2001-01-01', '2024-01-01');
insert into trained_in values (8, 2, '2000-01-01', '2025-01-01');
insert into trained_in values (1, 2, '2000-01-01', '2019-01-01');
insert into trained_in values (6, 2, '2000-01-01', '2001-01-01');
insert into trained_in values (3, 2, '2002-01-01', '2018-01-01');

insert into undergoes values (64, 5, 3, '2022-06-25', 7, 12);
insert into undergoes values (32, 3, 6, '2023-01-15', 1, NULL);
insert into undergoes values (32, 4, 6, '2023-01-15', 2, 10);
insert into undergoes values (32, 2, 8, '2023-01-22', 6, 11);
insert into undergoes values (59, 2, 9, '2022-10-05', 7, 14);

insert into appointment values (1, 11, 9, 5, '2022-04-19 07:00:00', '2022-04-19 07:30:00', 'D442');
insert into appointment values (2, 59, 14, 5, '2022-08-01 07:00:00', '2022-08-01 07:30:00', 'D442');
insert into appointment values (3, 59, 13, 5, '2022-12-13 08:00:00', '2022-12-13 08:30:00', 'D442');
insert into appointment values (4, 102, NULL, 3, '2023-01-09 12:00:00', '2022-12-13 12:30:00', 'NC442');
insert into appointment values (5, 32, 15, 1, '2023-01-15: 20:00:00', '2023-01-15: 20:30:00', '2.2');
insert into appointment values (6, 32, 10, 2, '2023-01-15: 19:00:00', '2023-01-15: 19:30:00', 'TSG');
insert into appointment values (7, 32, 12, 3, '2023-01-20: 19:00:00', '2023-01-20: 19:30:00', 'Dep');

insert into medication values (1, 'remdesivir', 'BIG Pharma', 'text');
insert into medication values (2, 'copium', 'Small Pharma', 'text');
insert into medication values (3, 'rat poison', 'Local Store', 'text');
insert into medication values (4, 'assignment', 'Professor House', 'text');

insert into prescribes values (6, 32, 1, '2022-05-01', 4, '1D');
insert into prescribes values (3, 102, 4, '2022-01-09', 4, '1D');
insert into prescribes values (5, 11, 1, '2022-04-19', 1, '2D');
insert into prescribes values (5, 59, 1, '2022-08-01', 2, '1D');
insert into prescribes values (5, 59, 1, '2022-12-13', 3, '2D');
insert into prescribes values (1, 32, 2, '2023-01-15', 5, '4D');
insert into prescribes values (2, 32, 2, '2023-01-15', 6, '2D');

-- Queries

-- Q1

select name 
from physician 
where employeeid in 
(
	select physician 
	from trained_in
	where treatment in 
	(
		select code 
		from procedure 
		where lower(name)='bypass surgery'
	)
);

-- Q2

select name 
from physician 
where employeeid in 
(
	select physician 
	from trained_in 
	where treatment = 
	(
		select code 
		from procedure 	
		where lower(name)='bypass surgery'
	)
) 
and employeeid in 
(
	select physician 
	from affiliated_with 
	where department = 
	(
		select departmentid 
		from department 
		where lower(name) = 'cardiology'
	)
);

-- Q3

select name 
from nurse
where employeeid in
(
	select nurse 
	from on_call
	where (blockfloor, blockcode) = 
	(
		select blockfloor, blockcode
		from room
		where number = 123
	)
);

-- Q4

select name, address
from patient
where ssn in
(
	select patient
	from prescribes
	where medication = 
	(
		select code 
		from medication
		where lower(name) = 'remdesivir'
	)
);

-- Q5

select name, insuranceid 
from patient
where ssn in
(
	select patient
	from
	(
		select patient, ("End" - start) as total
		from stay
		where room in
		(
			select number 
			from room
			where lower(type) = 'icu'
		)
	) as sq
	where extract(EPOCH FROM sq.total)/3600/24 > 15
);

-- Q6

select name 
from nurse
where employeeid in
(
	select assistingnurse
	from undergoes
	where procedure = 
	(
		select code 
		from procedure 	
		where lower(name)='bypass surgery'
	)
);

-- Q7

select nurse.name, nurse.position, physician.name
from nurse, physician
where (nurse.employeeid, physician.employeeid) in
(
	select assistingnurse, physician
	from undergoes
	where procedure = 
	(
		select code 
		from procedure 	
		where lower(name)='bypass surgery'
	)
);

-- Q8

select name 
from physician
where employeeid in
(
	select physician
	from undergoes
	where (physician, procedure) not in
	(
		select physician, treatment
		from trained_in
	)
);

-- Q9

select name 
from physician
where employeeid in
(
	select physician
	from undergoes
	where (physician, procedure) in
	(
		select physician, treatment
		from trained_in
		where certificationexpires < undergoes.date
	)
);

-- Q10

with treatmentgonewrong as
(
	select physician, procedure, date, patient
	from undergoes
	where (physician, procedure) in
	(
		select physician, treatment
		from trained_in
		where certificationexpires < undergoes.date
	)
)
select physician.name, procedure.name, treatmentgonewrong.date, patient.name
from physician, treatmentgonewrong, procedure, patient
where treatmentgonewrong.physician = physician.employeeid and treatmentgonewrong.procedure = procedure.code and patient.ssn = treatmentgonewrong.patient;

-- Q11

select patient.name, physician.name 
from patient, physician
where patient.ssn in
(
	select patient
	from prescribes
)
and patient.ssn in
(
	select patient
	from undergoes
	where procedure in
	(
		select code 
		from procedure
		where cost > 5000
	)
)
and pcp not in
(
	select head 
	from department
)
and patient.ssn in
(
	with pd as
	(
		select patient, count(patient) as cnt
		from appointment
		where physician in
		(
			select physician 
			from affiliated_with
			where department =
			(
				select departmentid
				from department
				where lower(name) = 'cardiology'
			)
		)
		group by patient
	)
	select patient
	from pd
	where cnt > 1
)
and pcp = physician.employeeid;

-- Q12

select name, brand
from medication
where code = 
(
	select medication
	from prescribes
	group by medication
	order by count(medication) desc 
	limit 1
);


