C----------------------------------------------------------------------
      SUBROUTINE RCLOS()
C     DUMMY IF HBOOK IS USED
C----------------------------------------------------------------------
      END


C----------------------------------------------------------------------
      SUBROUTINE HWABEG
C     USER'S ROUTINE FOR INITIALIZATION
C----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
      REAL*4 pi
      parameter (pi=3.14160E0)
      integer j,k
c

      call inihist
      k=0
      call mbook(k+ 1,'t pt',4.e0,0.e0,400.e0)
      call mbook(k+ 2,'t eta',0.2e0,-9.e0,9.e0)
      call mbook(k+ 3,'t log[pt]',0.05e0,0.1e0,5.e0)
      call mbook(k+ 4,'t y',0.2e0,-9.e0,9.e0)

      END


C----------------------------------------------------------------------
      SUBROUTINE HWAEND
C     USER'S ROUTINE FOR TERMINAL CALCULATIONS, HISTOGRAM OUTPUT, ETC
C----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
      REAL*8 XNORM
      INTEGER I,J,K
      OPEN(UNIT=99,FILE='HERST.TOP',STATUS='UNKNOWN')
C XNORM IS SUCH THAT THE CROSS SECTION PER BIN IS IN PB, SINCE THE HERWIG 
C WEIGHT IS IN NB, AND CORRESPONDS TO THE AVERAGE CROSS SECTION
      XNORM=1.D3/DFLOAT(NEVHEP)
      DO I=1,100              
 	CALL MFINAL3(I)             
        CALL MCOPY(I,I+100)
        CALL MOPERA(I+100,'F',I+100,I+100,SNGL(XNORM),0.E0)
 	CALL MFINAL3(I+100)             
      ENDDO                          
C
      k=0
      call multitop(100+k+ 1,99,3,2,'t pt',' ','LOG')
      call multitop(100+k+ 2,99,3,2,'t eta',' ','LOG')
      call multitop(100+k+ 3,99,3,2,'t log[pt]',' ','LOG')
      call multitop(100+k+ 4,99,3,2,'t y',' ','LOG')
c
      CLOSE(99)
      END


C----------------------------------------------------------------------
      SUBROUTINE HWANAL
C     USER'S ROUTINE TO ANALYSE DATA FROM EVENT
C----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
      DOUBLE PRECISION HWVDOT,PSUM(4)
      INTEGER ICHSUM,ICHINI,IHEP
      LOGICAL DIDSOF,sicuts,flcuts
      INTEGER ID,ID1,IST,IQ,IT1,ILP,INU,IBQ,IJ
      DOUBLE PRECISION YCUT,PTCUT,pt1,eta1,getpseudorap,yt1,
     # getrapidity,ptlp,ylp,ptnu,ynu,ptbq,ybq,xmw1,getinvm
      DOUBLE PRECISION XPTQ(5),XPLP(5),XPNU(5),XPBQ(5),YPW1(5)
      REAL*8 PI
      PARAMETER (PI=3.14159265358979312D0)
      REAL*8 WWW0
      INTEGER KK,JJ
c
      IF(IERROR.NE.0)RETURN
c
C INCOMING PARTONS MAY TRAVEL IN THE SAME DIRECTION: IT'S A POWER-SUPPRESSED
C EFFECT, SO THROW THE EVENT AWAY
      IF(SIGN(1.D0,PHEP(3,4)).EQ.SIGN(1.D0,PHEP(3,5)))THEN
         write(*,*)(phep(ij,1),ij=1,4)
         write(*,*)(phep(ij,2),ij=1,4)
        CALL HWWARN('HWANAL',111)
        GOTO 999
      ENDIF
      WWW0=EVWGT
      CALL HWVSUM(4,PHEP(1,1),PHEP(1,2),PSUM)
      CALL HWVSCA(4,-1D0,PSUM,PSUM)
      ICHSUM=0
      ICHINI=ICHRG(IDHW(1))+ICHRG(IDHW(2))
      DIDSOF=.FALSE.
      IQ=0
      DO 100 IHEP=1,NHEP
        IF (IDHW(IHEP).EQ.16) DIDSOF=.TRUE.
        IF (ISTHEP(IHEP).EQ.1) THEN
          CALL HWVSUM(4,PHEP(1,IHEP),PSUM,PSUM)
          ICHSUM=ICHSUM+ICHRG(IDHW(IHEP))
        ENDIF
        IST=ISTHEP(IHEP)
        ID1=IDHEP(IHEP)
        IF((IST.EQ.155.OR.IST.EQ.1).AND.ID1.EQ.6)THEN
C FOUND A TOP; KEEP ONLY THE FIRST ON RECORD
          IQ=IQ+1
          IF(IQ.EQ.1)IT1=IHEP
        ELSEIF((IST.EQ.155.OR.IST.EQ.1).AND.ID1.EQ.-6)THEN
C FOUND AN ANTITOP; KEEP ONLY THE FIRST ON RECORD
          IQ=IQ+1
          IF(IQ.EQ.1)IT1=IHEP
        ENDIF
  100 CONTINUE
      IF(IQ.EQ.0)THEN
        WRITE(*,*)"NO TOP NOR ANTITOP FOUND"
        STOP
      ENDIF  
C CHECK MOMENTUM AND CHARGE CONSERVATION
      IF (HWVDOT(3,PSUM,PSUM).GT.1.E-4*PHEP(4,1)**2) THEN
         CALL HWUEPR
         CALL HWWARN('HWANAL',112)
         GOTO 999
      ENDIF
      IF (ICHSUM.NE.ICHINI) THEN
         CALL HWUEPR
         CALL HWWARN('HWANAL',113)
         GOTO 999
      ENDIF
C FILL THE FOUR-MOMENTA
      DO IJ=1,5
        XPTQ(IJ)=PHEP(IJ,IT1)
      ENDDO
C FILL THE HISTOS
      YCUT=2.5D0
c
      pt1=sqrt(xptq(1)**2+xptq(2)**2)
      eta1=getpseudorap(xptq(4),xptq(1),xptq(2),xptq(3))
      yt1=getrapidity(xptq(4),xptq(3))
C
      kk=0
      call mfill(kk+1,sngl(pt1),sngl(WWW0))
      call mfill(kk+2,sngl(eta1),sngl(WWW0))
      if(pt1.gt.0.d0)call mfill(kk+3,sngl(log10(pt1)),sngl(WWW0))
      call mfill(kk+4,sngl(yt1),sngl(WWW0))
 999  RETURN
      END


      function getrapidity(en,pl)
      implicit none
      real*8 getrapidity,en,pl,tiny,xplus,xminus,y
      parameter (tiny=1.d-5)
c
      xplus=en+pl
      xminus=en-pl
      if(xplus.gt.tiny.and.xminus.gt.tiny)then
        if( (xplus/xminus).gt.tiny )then
          y=0.5d0*log( xplus/xminus )
        else
          y=sign(1.d0,pl)*1.d8
        endif
      else
        y=sign(1.d0,pl)*1.d8
      endif
      getrapidity=y
      return
      end


      function getpseudorap(en,ptx,pty,pl)
      implicit none
      real*8 getpseudorap,en,ptx,pty,pl,tiny,pt,eta,th
      parameter (tiny=1.d-5)
c
      pt=sqrt(ptx**2+pty**2)
      if(pt.lt.tiny.and.abs(pl).lt.tiny)then
        eta=sign(1.d0,pl)*1.d8
      else
        th=atan2(pt,pl)
        eta=-log(tan(th/2.d0))
      endif
      getpseudorap=eta
      return
      end


      function getinvm(en,ptx,pty,pl)
      implicit none
      real*8 getinvm,en,ptx,pty,pl,tiny,tmp
      parameter (tiny=1.d-5)
c
      tmp=en**2-ptx**2-pty**2-pl**2
      if(tmp.gt.0.d0)then
        tmp=sqrt(tmp)
      elseif(tmp.gt.-tiny)then
        tmp=0.d0
      else
        write(*,*)'Attempt to compute a negative mass'
        stop
      endif
      getinvm=tmp
      return
      end


      function getdelphi(ptx1,pty1,ptx2,pty2)
      implicit none
      real*8 getdelphi,ptx1,pty1,ptx2,pty2,tiny,pt1,pt2,tmp
      parameter (tiny=1.d-5)
c
      pt1=sqrt(ptx1**2+pty1**2)
      pt2=sqrt(ptx2**2+pty2**2)
      if(pt1.ne.0.d0.and.pt2.ne.0.d0)then
        tmp=ptx1*ptx2+pty1*pty2
        tmp=tmp/(pt1*pt2)
        if(abs(tmp).gt.1.d0+tiny)then
          write(*,*)'Cosine larger than 1'
          stop
        elseif(abs(tmp).ge.1.d0)then
          tmp=sign(1.d0,tmp)
        endif
        tmp=acos(tmp)
      else
        tmp=1.d8
      endif
      getdelphi=tmp
      return
      end
